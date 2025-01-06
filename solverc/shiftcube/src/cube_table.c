#include "shift_cube.h"
#include "solver_print.h"
#include "cube_table.h"
#include "lookup_tables.h"

typedef struct {
    shift_cube_s key;
    alg_list_s algs;
} entry_s;

typedef struct cube_table {
    size_t entries;
    size_t size;

    entry_s *table;
} cube_table_s;

cube_table_s* cube_table_create(size_t size) {
    cube_table_s *ct = (cube_table_s*)malloc(sizeof(cube_table_s));

    ct->table = (entry_s*)calloc(size, sizeof(entry_s));

    ct->entries = 0;
    ct->size    = size;
    return ct;
}

size_t cube_table_hash(const cube_table_s *ct, const shift_cube_s *key) {
    size_t hash = 0;
    for (face_e face = FACE_U; face < NUM_FACES; face++) {
        hash ^= key->state[face];
        hash  = rolq(hash, 17);
    }

    return hash % ct->size;
}

bool cube_table_insert(cube_table_s *ct, const shift_cube_s *key, const alg_s *moves) {
    if (ct == NULL || key == NULL || moves == NULL) {
        return false;
    }

    size_t hash = cube_table_hash(ct, key);
    size_t index = hash;

    // linear probing
    while (ct->table[index].algs.list != NULL) {
        if (compare_cubes(&(ct->table[index].key), key)) {
            if (ct->table[index].algs.num_algs == ct->table[index].algs.size) {
                size_t new_size = 2*sizeof(alg_s)*ct->table[index].algs.size;
                alg_s *tmp = realloc(ct->table[index].algs.list, new_size);

                if (!tmp) {
                    return false;
                }

                ct->table[index].algs.list = tmp;
                ct->table[index].algs.size *=2;
            }

            ct->table[index].algs.list[ct->table[index].algs.num_algs] = alg_static_copy(moves);
            ct->table[index].algs.num_algs++;

            return true;
        }

        index++;
        if (index == hash) {
            return false;
        }

        if (index >= ct->size) {
            index = 0;
        }
    }


    ct->table[index].key  = *key;

    ct->entries++;

    ct->table[index].algs.list = (alg_s*)malloc(sizeof(alg_s));
    ct->table[index].algs.list[0] = alg_static_copy(moves);

    ct->table[index].algs.num_algs = 1;
    ct->table[index].algs.size = 1;

    return true;
}

static inline size_t cube_table_get_cube_index(const cube_table_s *ct, const shift_cube_s *cube) {
    if (ct == NULL || cube == NULL) {
        return ct->size;
    }

    size_t hash = cube_table_hash(ct, cube);
    size_t index = hash;

    while (ct->table[index].algs.list != NULL) {
        if (compare_cubes(&(ct->table[index].key), cube)) {
            return index;
        }

        if (index + 1 == hash) {
            break;
        }

        index = (index + 1 >= ct->size) ? 0 : index + 1;
    }

    return ct->size;
}

const alg_list_s* cube_table_lookup(const cube_table_s *ct, const shift_cube_s *cube) {
    size_t index = cube_table_get_cube_index(ct, cube);

    return (index == ct->size) ? NULL : &ct->table[index].algs;
}

void cube_table_clear(cube_table_s *ct) {
    if (ct == NULL || ct->table == NULL) {
        return;
    }

    for (size_t index = 0; index < ct->size; index++) {
        if (ct->table[index].algs.list != NULL) {
            for (size_t i = 0; i < ct->table[index].algs.num_algs; i++) {
                free(ct->table[index].algs.list[i].moves);
            }

            free(ct->table[index].algs.list);
            ct->table[index].algs.list = NULL;
        }
    }

    ct->entries = 0;
}

void cube_table_free(cube_table_s *ct) {
    if (ct == NULL || ct->table == NULL) {
        free(ct);
        return;
    }

    cube_table_clear(ct);

    free(ct->table);
    free(ct);
}

void cube_table_print(cube_table_s *ct) {
    printf("   index  |               cube string representation            | algorithm\n");
    printf("------------------------------------------------------------------------------\n");
    for (size_t idx = 0; idx < ct->size; idx++) {
        if (ct->table[idx].algs.list != NULL) {
            printf("%10zu ", idx);
            print_cube_line_colors(ct->table[idx].key);
            print_alg(&ct->table[idx].algs.list[0]);
        }

        if (idx != 0) {
            if (ct->table[idx].algs.list == NULL && ct->table[idx-1].algs.list != NULL) {
                printf("                                    ...                                    \n");
            }
        }
    }
}

size_t cube_table_entries(const cube_table_s *ct) {
    return ct->entries;
}

size_t cube_table_size(const cube_table_s *ct) {
    return ct->size;
}

void cube_table_print_algs_bigger_than_n(const cube_table_s* ct, size_t n) {
    printf("   index  |               cube string representation            | algorithm\n");
    printf("------------------------------------------------------------------------------\n");
    for (size_t idx = 0; idx < ct->size; idx++) {
        if (ct->table[idx].algs.list[0].length <= n) continue;
        if (ct->table[idx].algs.list != NULL) {
            printf("%10zu ", idx);
            print_cube_line_colors(ct->table[idx].key);
            print_alg(&ct->table[idx].algs.list[0]);
        }
    }
}

bool cube_table_check_if_1LLL_is_valid(const cube_table_s* ct) {
    bool is_valid = true;
    if (cube_table_entries(ct) != 62208) is_valid = false;
    for (size_t idx = 0; idx < ct->size; idx++) {
        if (ct->table[idx].algs.list != NULL) {
            if (ct->table[idx].algs.num_algs != 1) is_valid = false;
            shift_cube_s cube_mask = masked_cube(&ct->table[idx].key, &f2l_4mask);
            shift_cube_s solved_mask = masked_cube(&SOLVED_SHIFTCUBE, &f2l_4mask);
            if (!compare_cubes(&cube_mask, &solved_mask)) is_valid = false;
        }
    } if (!is_valid) printf("1LLL NOT VALID!\n");
    return is_valid;
}

uint8_t LL_table_get_maxmimum_alg_length(const cube_table_s* ct) {
    uint8_t max = 0;
    for (size_t idx = 0; idx < ct->size; idx++) {
        if (ct->table[idx].algs.list != NULL) {
            uint8_t candidate = ct->table[idx].algs.list[0].length;
            if (candidate > max) max = candidate;
        }
    } return max;
}

void LL_table_diagnostics(const cube_table_s* ct) {
    if (cube_table_check_if_1LLL_is_valid(ct)) {
        printf("1LLL is valid!\n");
    }
    uint8_t maxlength = LL_table_get_maxmimum_alg_length(ct);
    size_t counts[maxlength+1];
    for (int i = 0; i < maxlength+1; i++) counts[i] = 0;

    for (size_t idx = 0; idx < ct->size; idx++) {
        if (ct->table[idx].algs.list != NULL) {
            counts[ct->table[idx].algs.list[0].length]++;
        }
    }
    printf("maximum length is %hhu", maxlength);
    printf("\nlengths:    ");
    for (uint8_t i = 0; i < maxlength+1; i++) {
        printf("%6hhu", i);
    }
    printf("\nfrequency: [");
    for (uint8_t i = 0; i < maxlength+1; i++) {
        printf("%6zu", counts[i]);
    } printf("]\n");
}