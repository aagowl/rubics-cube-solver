#include "alg.h"

alg_s *alg_create(size_t size) {
    alg_s *alg = (alg_s*)malloc(sizeof(alg_s));

    alg->moves = (move_e*)malloc(size * sizeof(move_e));
    alg->length = 0;
    alg->size = size;

    return alg;
}
// return a pointer to a new alg_s with a distinct yet identical array of the alg from src
alg_s* alg_copy(const alg_s *src) {
    if (!src) return NULL;

    alg_s *copy = alg_create(src->length);

    (void)memcpy(copy->moves, src->moves, src->length * sizeof(move_e));
    copy->length = src->length;

    return copy;
}

alg_s alg_static_copy(const alg_s *src) {
    alg_s copy;
    copy.moves = malloc(sizeof(move_e) * src->length);
    (void)memcpy(copy.moves, src->moves, src->length * sizeof(move_e));
    copy.length = src->length;
    copy.size   = src->length;

    return copy;
}

void alg_free(alg_s *alg) {
    if (alg == NULL) {
        return;
    }

    free(alg->moves);
    free(alg);
}

bool alg_insert(alg_s *alg, move_e move, size_t index) {
    // don't try to insert if index is out of bounds
    if (index > alg->length) {
        return false;
    }

	// reallocate the move moves if needed
    if (alg->length == alg->size) {
        alg->size *= 2;
        alg->moves = (move_e*)realloc(alg->moves,
                                       alg->size * sizeof(move_e));
    }

    // if this isn't an append, move everything after index one space up
    if (index != alg->length) {
        (void)memmove(alg->moves + index + 1, alg->moves + index,
                      (alg->length - index) * sizeof(move_e));
    }

    alg->moves[index] = move;
    alg->length++;

    // insertion was successful
    return true;
}

bool alg_delete(alg_s *alg, size_t index) {
    // don't try to delete if index is out of bounds
    if (index >= alg->length) {
        return false;
    }

    // if this isn't just a "pop", bring alg ahead of index one index down
    // and if it is, there's no need to do anything besides decrement length
    if (index != alg->length - 1) {
        (void)memmove(alg->moves + index, alg->moves + index + 1,
                      sizeof(move_e) * (alg->length - (index + 1)));
    }
    alg->length--;

    // for memory leak protection: decrease the size of the move moves if length <= 1/4 size
    // and the length is still greater than INIT_alg_SIZE
    if (alg->length >= MIN_LIST_RESIZE && alg->length <= alg->size/4) {
        alg->size /= 2;
        alg->moves = (move_e*)realloc(alg->moves, sizeof(move_e) * alg->size);
    }

    // deletion was successful
    return true;
}

// reverse a alg_s "alg" in place
void alg_invert(alg_s *alg) {
    if (alg == NULL) { 
        return;
    }

    if (alg->moves == NULL || alg->length == 0 || alg->size == 0) {
        return;
    }

    if (alg->length == 1) {
        //alg->moves[0].turns = mod4(-alg->moves[0].turns);
        alg->moves[0] = move_inverted[alg->moves[0]];
        return;
    }

    for (size_t i = 0, j = alg->length-1; i <= j; i++, j--) {
        move_e tmp  = move_inverted[alg->moves[i]];
        move_e tmp2 = move_inverted[alg->moves[j]];
        alg->moves[i] = tmp2;
        alg->moves[j] = tmp;
    }
}

size_t alg_lookup(const alg_s *alg, move_e move) {
    for (size_t i = 0; i < alg->length; i++) {
        if (alg->moves[i] == move) return i;
    } return -1;
}

// simplify move sequences in the move moves
void alg_simplify(alg_s *alg) {
    if (alg == NULL) {
        return;
    }

    size_t idx = 0;
    size_t idx2 = 1;
    while (idx2 < alg->length) {
        while (move_faces[alg->moves[idx2]] == opposite_faces[move_faces[alg->moves[idx]]]
               && idx2 < alg->length - 1 && idx2 > 0) {
            idx2 += 1;
        }

        while (move_faces[alg->moves[idx]] == move_faces[alg->moves[idx2]]) {
            alg->moves[idx] = move_e_add(alg->moves[idx], alg->moves[idx2]);
            //alg->moves[idx].turns += alg->moves[idx2].turns;
            alg_delete(alg, idx2);

            // if we can't delete any more alg after this, stop
            if (idx2 >= alg->length || idx >= alg->length) {
                break;
            }
        }

        /* if our combined move consititutes zero alg, get rid of it
         * and if we can, check the move before this deleted move for
         * simplification. Additionally, if there's a sequence of
         * consequtive same/opposite face alg, keep moving idx back
         * to account for new potential simplifications of earlier alg.
         */
        if (alg->moves[idx] == MOVE_NULL) {
            alg_delete(alg, idx);
            while (--idx > 0) {
                if (!(move_faces[alg->moves[idx]] == opposite_faces[move_faces[alg->moves[idx - 1]]] ||
                    move_faces[alg->moves[idx]] == move_faces[alg->moves[idx-1]])) {
                    break;
                }
            }
            idx2 = idx + 1;
            continue;
        }

        idx++;
        idx2 = idx + 1;
    }
}

///////////////////////////// MUST CONVERT TO MOVE_E /////////////////////////////
alg_s* alg_from_alg_str(const char *alg_str) {
    if (alg_str == NULL) {
        return alg_create(0);
    }

    // This will get the length of the string.
    size_t len = 0;
    while (alg_str[len]) len++;

    // get faces
    face_e chars_to_faces[128] = { FACE_NULL };
    chars_to_faces['U'] = FACE_U;
    chars_to_faces['R'] = FACE_R;
    chars_to_faces['F'] = FACE_F;
    chars_to_faces['L'] = FACE_L;
    chars_to_faces['B'] = FACE_B;
    chars_to_faces['D'] = FACE_D;

    // declare alg (init not done yet)
    alg_s *alg = alg_create(2 * MIN_LIST_RESIZE);

    size_t idx = 0;
    move_e move = MOVE_NULL;
    while (idx < len) {
        uint8_t c = alg_str[idx];
        if (c == ' ' || c == '\n' || c == '\t') {
            idx++;
            continue;
        };

        face_e face = chars_to_faces[c];
        if (face == FACE_NULL) {
            printf("AAAAAAAAAA, alg_from_alg_str goofed up!: %s", alg_str);
            break;
        } else {
            if (idx+1 == len) {
                move = face*3;
                idx++;
            } else if (alg_str[idx+1] == '2') {
                move = face*3 + 1;
                idx += 2;
            } else if (alg_str[idx+1] == '\'' || alg_str[idx+1] == '3') {
                move = face*3 + 2;
                idx += 2;
            } else {
                move = face*3;
                idx++;
            } 
            alg_insert(alg, move, alg->length);
        }
    }
    return alg;
}

void alg_rotate_on_y(alg_s *alg, uint8_t y_turns) {
    if (alg == NULL || alg->moves == NULL) {
        return;
    }

    for (int i = 0; i < alg->length; i++) {
        alg->moves[i] = move_rotated_on_y[mod4(y_turns)][alg->moves[i]];
    }
}

move_e* alg_concat(alg_s *dest, const alg_s *src) {
    if (!dest || !src) {
        return NULL;
    }

    size_t new_len = dest->length + src->length;
    if (new_len > dest->size) {
        move_e *tmp = (move_e*)realloc(dest->moves, sizeof(move_e)*new_len);
        if (!tmp) {
            return NULL;
        }
        dest->moves = tmp;
        dest->size = new_len;
    }

    for (size_t idx = 0; idx < src->length; idx++) {
        dest->moves[dest->length+idx] = src->moves[idx];
    }

    dest->length = new_len;
    return dest->moves;
}

alg_list_s* alg_list_create(size_t size) {
    alg_list_s* alg_list = (alg_list_s*)malloc(sizeof(alg_list_s));

    alg_list->list = (alg_s*)malloc(size * sizeof(alg_s));
    alg_list->num_algs = 0;
    alg_list->size = size;

    return alg_list;
}
void alg_list_append(alg_list_s *alg_list, const alg_s* alg) {
    if (alg_list->num_algs == alg_list->size) {
        alg_list->size *= 2;
        alg_list->list = (alg_s*)realloc(alg_list->list,
                                              alg_list->size * sizeof(alg_s));
    }
    alg_list->list[alg_list->num_algs] = alg_static_copy(alg);
    alg_list->num_algs++;
}
void alg_list_free(alg_list_s *alg_list) {
    for (size_t index = 0; index < alg_list->num_algs; index++) {
        free(alg_list->list[index].moves);
    }
    free(alg_list->list);
    free(alg_list);
}