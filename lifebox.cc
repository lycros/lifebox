// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// This code is public domain
// (but note, once linked against the led-matrix library, this is
// covered by the GPL v2)
//
// This is a grab-bag of various demos and not very readable.
#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "pixel-mapper.h"
#include "graphics.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <random>

using std::random_device;
using std::mt19937;

using rgb_matrix::RGBMatrix;
using rgb_matrix::PrintMatrixFlags;
using rgb_matrix::RuntimeOptions;

using rgb_matrix::ThreadedCanvasManipulator;
using rgb_matrix::Canvas;
using rgb_matrix::Color;

volatile bool interrupt_received = false;

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

static bool parseColor(Color *c, const char *str) {
    return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

class LifeBox : public ThreadedCanvasManipulator {
private:
    int delay_ms_;
    int width_;
    int height_;

    struct Plant {
        int age;
        int energy;
    };

    struct Specie {
        int age;
        int energy;
    };

    Plant** plantes_;
    Specie** species1_;
    Specie** species2_;

    Color* color_plantes_;
    Color* color_species_comp_;
    Color* color_species1_;
    Color* color_species2_;
    Color* color_nothing_;

    enum {
        PLANTS_LIFE_EXPECTANCY = 100,
        PLANTS_RANDOM_BORN_CHANCES = 1200, // high is less chances
        PLANTS_RANDOM_NEARBORN_CHANCES = 120,
        PLANTS_RANDOM_DIE_CHANCES = 2,
        PLANTS_ENERGY_BASE_PER_CYCLE = 5,

        SPECIE1_LIFE_EXPECTANCY = 200,
        SPECIE1_RANDOM_BORN_CHANCES = 5000,
        SPECIE1_RANDOM_NEARBORN_CHANCES = 50,
        SPECIE1_RANDOM_DIE_CHANCES = 2,
        SPECIE1_ENERGY_BASE = 30,
        SPECIE1_ENERGY_NEEDED_PER_CYCLE = 2,
        SPECIE1_MAX_ENERGY_RECOLECTED_PER_CYCLE = 30,
        SPECIE1_ENERGY_TO_REPLICATE = 10,

        SPECIE2_LIFE_EXPECTANCY = 170,
        SPECIE2_RANDOM_BORN_CHANCES = 4000,
        SPECIE2_RANDOM_NEARBORN_CHANCES = 40,
        SPECIE2_RANDOM_DIE_CHANCES = 2,
        SPECIE2_ENERGY_BASE = 25,
        SPECIE2_ENERGY_NEEDED_PER_CYCLE = 2,
        SPECIE2_MAX_ENERGY_RECOLECTED_PER_CYCLE = 25,
        SPECIE2_ENERGY_TO_REPLICATE = 5
    };
public:

    LifeBox(Canvas *m, int delay_ms = 500) : ThreadedCanvasManipulator(m) {
        
        delay_ms_ = delay_ms;
        width_ = canvas()->width();
        height_ = canvas()->height();

        // Clear the board
        plantes_ = new Plant*[width_];
        species1_ = new Specie*[width_];
        species2_ = new Specie*[width_];

        for (int x = 0; x < width_; ++x) {
            plantes_[x] = new Plant[height_];
            species1_[x] = new Specie[height_];
            species2_[x] = new Specie[height_];
        }

        color_plantes_ = new Color(255, 255, 255);
        color_species_comp_ = new Color(255, 0, 255);
        color_species1_ = new Color(255, 255, 0);
        color_species2_ = new Color(0, 255, 255);
        color_nothing_ = new Color(0, 0, 0);

    }

    ~LifeBox() {
        for (int x = 0; x < width_; ++x) {
            delete[] plantes_[x];
            delete[] species1_[x];
            delete[] species2_[x];
        }

        delete[] plantes_;
        delete[] species1_;
        delete[] species2_;

        delete color_plantes_;
        delete color_species_comp_;
        delete color_species1_;
        delete color_species2_;
        delete color_nothing_;
    }

    void Run() {
        random_device device;
        mt19937 mt_rand(device());

        int x, y, xp, xm, yp, ym;
        int plants_neighbours, specie1_neighbours;
        int specie2_neighbours, pos;
        int random_number, rand_pos;
        int total_energy;
        int available[8];

        while (running()) {

            for (x = 0; x < width_; ++x) {
                xp = (x + 1) & (width_ - 1);
                xm = (x - 1) & (width_ - 1);
                for (y = 0; y < height_; ++y) {
                    yp = (y + 1) & (height_ - 1);
                    ym = (y - 1) & (height_ - 1);
                    // Count the number of currently live neighbouring cells
                    plants_neighbours = 0;
                    specie1_neighbours = 0;
                    specie2_neighbours = 0;
                    // [Plants]
                    if (plantes_[x][y].age == 0 && plantes_[xm][y].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[xp][y].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[xm][ym].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[x][ym].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[xp][ym].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[xm][yp].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[x][yp].age > 0) {
                        plants_neighbours++;
                    }
                    if (plantes_[x][y].age == 0 && plantes_[xp][yp].age > 0) {
                        plants_neighbours++;
                    }
                    // [Specie1]
                    if (species1_[x][y].age == 0 && species1_[xm][y].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[xp][y].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[xm][ym].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[x][ym].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[xp][ym].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[xm][yp].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[x][yp].age > 0) {
                        specie1_neighbours++;
                    }
                    if (species1_[x][y].age == 0 && species1_[xp][yp].age > 0) {
                        specie1_neighbours++;
                    }
                    // [Specie2]
                    if (species2_[x][y].age == 0 && species2_[xm][y].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[xp][y].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[xm][ym].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[x][ym].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[xp][ym].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[xm][yp].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[x][yp].age > 0) {
                        specie2_neighbours++;
                    }
                    if (species2_[x][y].age == 0 && species2_[xp][yp].age > 0) {
                        specie2_neighbours++;
                    }

                    // [Plants logic]
                    // plant dies
                    if (plantes_[x][y].age >= PLANTS_LIFE_EXPECTANCY) {
                        plantes_[x][y].age = 0;
                        plantes_[x][y].energy = 0;
                        // plant dies
                    } else if (plantes_[x][y].age > 0 && plantes_[x][y].age < PLANTS_LIFE_EXPECTANCY && plantes_[x][y].energy <= 0) {
                        plantes_[x][y].age = 0;
                        plantes_[x][y].energy = 0;
                        // plant grows
                    } else if (plantes_[x][y].age > 0 && plantes_[x][y].age < PLANTS_LIFE_EXPECTANCY) {
                        plantes_[x][y].age++;
                        plantes_[x][y].energy = plantes_[x][y].energy + PLANTS_ENERGY_BASE_PER_CYCLE;
                        // no neighbours plant born
                    } else if (plantes_[x][y].age == 0 && plants_neighbours == 0) {
                        random_number = mt_rand() % PLANTS_RANDOM_BORN_CHANCES;
                        if (random_number == 1) {
                            plantes_[x][y].age = 1;
                            plantes_[x][y].energy = 1;
                        }
                        // neighbours plant born
                    } else if (plantes_[x][y].age == 0 && plants_neighbours > 0) {
                        random_number = mt_rand() % PLANTS_RANDOM_NEARBORN_CHANCES;
                        if (random_number == 1) {
                            plantes_[x][y].age = 1;
                            plantes_[x][y].energy = 1;
                        }
                    }

                    // Specie1 logic
                    if (species1_[x][y].age > 0) { // if there are an individual alive
                        // try to eat
                        if (plantes_[x][y].energy > 0) {
                            total_energy = 0;
                            if (plantes_[x][y].energy > SPECIE1_MAX_ENERGY_RECOLECTED_PER_CYCLE) {
                                total_energy = SPECIE1_MAX_ENERGY_RECOLECTED_PER_CYCLE;
                                plantes_[x][y].energy = plantes_[x][y].energy - SPECIE1_MAX_ENERGY_RECOLECTED_PER_CYCLE;
                            } else {
                                total_energy = plantes_[x][y].energy;
                                plantes_[x][y].energy = 0;
                            }
                            species1_[x][y].energy = species1_[x][y].energy + total_energy;
                        }
                        // grow and decrease energy
                        species1_[x][y].age++;
                        species1_[x][y].energy = species1_[x][y].energy - SPECIE1_ENERGY_NEEDED_PER_CYCLE;
                        // die
                        if (species1_[x][y].energy < 0) {
                            species1_[x][y].energy = 0;
                            species1_[x][y].age = 0;
                        } else if (species1_[x][y].energy > SPECIE1_ENERGY_TO_REPLICATE) {
                            memset(available, 0, sizeof (available));
                            pos = 0;

                            random_number = mt_rand() % SPECIE1_RANDOM_NEARBORN_CHANCES;
                            if (species1_[xm][y].age == 0) {
                                available[pos] = 1;
                                pos++;
                            }
                            if (species1_[xp][y].age == 0) {
                                available[pos] = 2;
                                pos++;
                            }
                            if (species1_[xm][ym].age == 0) {
                                available[pos] = 3;
                                pos++;
                            }
                            if (species1_[x][ym].age == 0) {
                                available[pos] = 4;
                                pos++;
                            }
                            if (species1_[xp][ym].age == 0) {
                                available[pos] = 5;
                                pos++;
                            }
                            if (species1_[xm][yp].age == 0) {
                                available[pos] = 6;
                                pos++;
                            }
                            if (species1_[x][yp].age == 0) {
                                available[pos] = 7;
                                pos++;
                            }
                            if (species1_[xp][yp].age == 0) {
                                available[pos] = 8;
                                pos++;
                            }

                            if (pos > 0) {
                                rand_pos = mt_rand() % pos;
                                switch (available[rand_pos]) { // one individual born radomly
                                    case 1:
                                        if (random_number == 1) {
                                            species1_[xm][y].age = 1;
                                            species1_[xm][y].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 2:
                                        if (random_number == 1) {
                                            species1_[xp][y].age = 1;
                                            species1_[xp][y].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 3:
                                        if (random_number == 1) {
                                            species1_[xm][ym].age = 1;
                                            species1_[xm][ym].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 4:
                                        if (random_number == 1) {
                                            species1_[x][ym].age = 1;
                                            species1_[x][ym].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 5:
                                        if (random_number == 1) {
                                            species1_[xp][ym].age = 1;
                                            species1_[xp][ym].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 6:
                                        if (random_number == 1) {
                                            species1_[xm][yp].age = 1;
                                            species1_[xm][yp].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 7:
                                        if (random_number == 1) {
                                            species1_[x][yp].age = 1;
                                            species1_[x][yp].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    case 8:
                                        if (random_number == 1) {
                                            species1_[xp][yp].age = 1;
                                            species1_[xp][yp].energy = SPECIE1_ENERGY_BASE;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                            }
                            // die if too old
                        } else if (species1_[x][y].age > SPECIE1_LIFE_EXPECTANCY) {
                            species1_[x][y].energy = 0;
                            species1_[x][y].age = 0;
                        }
                    } else if (species1_[x][y].age == 0) { // if theres no individual, new individual will born? (to avoid extintion)
                        if (specie1_neighbours == 0) {

                            random_number = mt_rand() % SPECIE1_RANDOM_BORN_CHANCES;
                            if (random_number == 1) {
                                species1_[x][y].age = 1;
                                species1_[x][y].energy = SPECIE1_ENERGY_BASE;
                            }
                        }
                    }
                    // Specie2 logic

                    if (species2_[x][y].age > 0) { // if there are an individual alive
                        // try to eat
                        if (plantes_[x][y].energy > 0) {
                            total_energy = 0;
                            if (plantes_[x][y].energy > SPECIE2_MAX_ENERGY_RECOLECTED_PER_CYCLE) {
                                total_energy = SPECIE2_MAX_ENERGY_RECOLECTED_PER_CYCLE;
                                plantes_[x][y].energy = plantes_[x][y].energy - SPECIE2_MAX_ENERGY_RECOLECTED_PER_CYCLE;
                            } else {
                                total_energy = plantes_[x][y].energy;
                                plantes_[x][y].energy = 0;
                            }
                            species2_[x][y].energy = species2_[x][y].energy + total_energy;
                        }
                        // grow and decrease energy
                        species2_[x][y].age++;
                        species2_[x][y].energy = species2_[x][y].energy - SPECIE2_ENERGY_NEEDED_PER_CYCLE;
                        if (species2_[x][y].energy < 0) {
                            species2_[x][y].energy = 0;
                            species2_[x][y].age = 0;
                        } else if (species2_[x][y].energy > SPECIE2_ENERGY_TO_REPLICATE) {
                            memset(available, 0, sizeof (available));
                            pos = 0;

                            random_number = mt_rand() % SPECIE2_RANDOM_NEARBORN_CHANCES;
                            if (species2_[xm][y].age == 0) {
                                available[pos] = 1;
                                pos++;
                            }
                            if (species2_[xp][y].age == 0) {
                                available[pos] = 2;
                                pos++;
                            }
                            if (species2_[xm][ym].age == 0) {
                                available[pos] = 3;
                                pos++;
                            }
                            if (species2_[x][ym].age == 0) {
                                available[pos] = 4;
                                pos++;
                            }
                            if (species2_[xp][ym].age == 0) {
                                available[pos] = 5;
                                pos++;
                            }
                            if (species2_[xm][yp].age == 0) {
                                available[pos] = 6;
                                pos++;
                            }
                            if (species2_[x][yp].age == 0) {
                                available[pos] = 7;
                                pos++;
                            }
                            if (species2_[xp][yp].age == 0) {
                                available[pos] = 8;
                                pos++;
                            }

                            if (pos > 0) {
                                rand_pos = mt_rand() % pos;
                                switch (available[rand_pos]) { // one individual born radomly
                                    case 1:
                                        if (random_number == 1) {
                                            species2_[xm][y].age = 1;
                                            species2_[xm][y].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 2:
                                        if (random_number == 1) {
                                            species2_[xp][y].age = 1;
                                            species2_[xp][y].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 3:
                                        if (random_number == 1) {
                                            species2_[xm][ym].age = 1;
                                            species2_[xm][ym].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 4:
                                        if (random_number == 1) {
                                            species2_[x][ym].age = 1;
                                            species2_[x][ym].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 5:
                                        if (random_number == 1) {
                                            species2_[xp][ym].age = 1;
                                            species2_[xp][ym].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 6:
                                        if (random_number == 1) {
                                            species2_[xm][yp].age = 1;
                                            species2_[xm][yp].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 7:
                                        if (random_number == 1) {
                                            species2_[x][yp].age = 1;
                                            species2_[x][yp].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    case 8:
                                        if (random_number == 1) {
                                            species2_[xp][yp].age = 1;
                                            species2_[xp][yp].energy = SPECIE2_ENERGY_BASE;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                            }
                        } else if (species2_[x][y].age > SPECIE2_LIFE_EXPECTANCY) {
                            species2_[x][y].energy = 0;
                            species2_[x][y].age = 0;
                        }
                    } else if (species2_[x][y].age == 0) { // if theres no individual, new individual will born? (to avoid extintion)
                        if (specie2_neighbours == 0) {
                            random_number = mt_rand() % SPECIE2_RANDOM_BORN_CHANCES;
                            if (random_number == 1) {
                                species2_[x][y].age = 1;
                                species2_[x][y].energy = SPECIE2_ENERGY_BASE;
                            }
                        }
                    }

                    // draw
                    if (species1_[x][y].age > 0 && species2_[x][y].age > 0) {
                        canvas()->SetPixel(x, y, color_species_comp_->r, color_species_comp_->g, color_species_comp_->b);
                    }// species comp
                    else if (species1_[x][y].age > 0 && species2_[x][y].age == 0) {
                        canvas()->SetPixel(x, y, color_species1_->r, color_species1_->g, color_species1_->b);
                    }// only specie1
                    else if (species1_[x][y].age == 0 && species2_[x][y].age > 0) {
                        canvas()->SetPixel(x, y, color_species2_->r, color_species2_->g, color_species2_->b);
                    }// only specie2
                    else if (species1_[x][y].age == 0 && species2_[x][y].age == 0 && plantes_[x][y].age > 0) {
                        canvas()->SetPixel(x, y, color_plantes_->r, color_plantes_->g, color_plantes_->b);
                    }// only plants
                    else if (species1_[x][y].age == 0 && species2_[x][y].age == 0 && plantes_[x][y].age == 0) {
                        canvas()->SetPixel(x, y, color_nothing_->r, color_nothing_->g, color_nothing_->b);
                    } // black nothing
                }
            }
            usleep(delay_ms_ * 1000); // ms
        }
    }

    void setPlantesColor(Color *color) {
        color_plantes_ = color;
    }

    void setSpeciesCompColor(Color *color) {
        color_species_comp_ = color;
    }

    void setSpecies1Color(Color *color) {
        color_species1_ = color;
    }

    void setSpecies2Color(Color *color) {
        color_species2_ = color;
    }

    void setNothingColor(Color *color) {
        color_nothing_ = color;
    }
};

static int usage(const char *progname) {
    fprintf(stderr, "usage: %s <options> [optional parameter]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr,
            "\t-t <seconds>      : Run for these number of seconds, then exit.\n"
            "\t-m <milliseconds> : Lifebox speed. Default 15.\n"
            "\t-V <r,g,b>        : Species-Comp-Color. Default 255,0,255\n"
            "\t-W <r,g,b>        : Plantes-Color. Default 255,255,255\n"
            "\t-X <r,g,b>        : Species1-Color, Default 255,255,0\n"
            "\t-Y <r,g,b>        : Species2-Color. Default 0,255,255\n"
            "\t-Z <r,g,b>        : Nothing-Color. Default 0,0,0\n");
    PrintMatrixFlags(stderr);
    return 1;
}

int main(int argc, char *argv[]) {
    int runtime_seconds = -1;
    int scroll_ms = 15;

    RGBMatrix::Options matrix_options;
    RuntimeOptions runtime_opt;

    // These are the defaults when no command-line flags are given.
    matrix_options.rows = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;


    Color* color_plantes = NULL;
    Color* color_species_comp = NULL;
    Color* color_species1 = NULL;
    Color* color_species2 = NULL;
    Color* color_nothing = NULL;

    // First things first: extract the command line flags that contain
    // relevant matrix options.
    if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return usage(argv[0]);
    }

    int opt;
    while ((opt = getopt(argc, argv, "t:m:V:W:X:Y:Z:")) != -1) {
        switch (opt) {
            case 't':
                runtime_seconds = atoi(optarg);
                break;
            case 'm':
                scroll_ms = atoi(optarg);
                break;
            case 'V':
                color_species_comp = new Color(0, 0, 0);
                if (!parseColor(color_species_comp, optarg)) {
                    fprintf(stderr, "Invalid color spec.\n");
                    return usage(argv[0]);
                }
                break;
            case 'W':
                color_plantes = new Color(0, 0, 0);
                if (!parseColor(color_plantes, optarg)) {
                    fprintf(stderr, "Invalid color spec.\n");
                    return usage(argv[0]);
                }
                break;
            case 'X':
                color_species1 = new Color(0, 0, 0);
                if (!parseColor(color_species1, optarg)) {
                    fprintf(stderr, "Invalid color spec.\n");
                    return usage(argv[0]);
                }
                break;
            case 'Y':
                color_species2 = new Color(0, 0, 0);
                if (!parseColor(color_species2, optarg)) {
                    fprintf(stderr, "Invalid color spec.\n");
                    return usage(argv[0]);
                }
                break;
            case 'Z':
                color_nothing = new Color(0, 0, 0);
                if (!parseColor(color_nothing, optarg)) {
                    fprintf(stderr, "Invalid color spec.\n");
                    return usage(argv[0]);
                }
                break;
            default: /* '?' */
                return usage(argv[0]);
        }
    }

    RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL)
        return 1;

    printf("Size: %dx%d. Hardware gpio mapping: %s\n",
            matrix->width(), matrix->height(), matrix_options.hardware_mapping);

    Canvas *canvas = matrix;

    // The ThreadedCanvasManipulator objects are filling
    // the matrix continuously.
    ThreadedCanvasManipulator *image_gen = NULL;
    LifeBox *lifebox = new LifeBox(canvas, scroll_ms);
    if (color_plantes != NULL) {
        lifebox->setPlantesColor(color_plantes);
    }
    if (color_species_comp != NULL) {
        lifebox->setSpeciesCompColor(color_species_comp);
    }
    if (color_species1 != NULL) {
        lifebox->setSpecies1Color(color_species1);
    }
    if (color_species2 != NULL) {
        lifebox->setSpecies2Color(color_species2);
    }
    if (color_nothing != NULL) {
        lifebox->setNothingColor(color_nothing);
    }
    image_gen = lifebox;

    if (image_gen == NULL)
        return usage(argv[0]);

    // Set up an interrupt handler to be able to stop animations while they go
    // on. Note, each demo tests for while (running() && !interrupt_received) {},
    // so they exit as soon as they get a signal.
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    // Image generating demo is crated. Now start the thread.
    image_gen->Start();

    // Now, the image generation runs in the background. We can do arbitrary
    // things here in parallel. In this demo, we're essentially just
    // waiting for one of the conditions to exit.
    if (runtime_seconds > 0) {
        sleep(runtime_seconds);
    } else {
        // The
        printf("Press <CTRL-C> to exit and reset LEDs\n");
        while (!interrupt_received) {
            sleep(1); // Time doesn't really matter. The syscall will be interrupted.
        }
    }
    
    image_gen->Stop();
    image_gen->WaitStopped();
    // Stop image generating thread. The delete triggers
    delete image_gen;
    delete canvas;

    printf("\%s. Exiting.\n", interrupt_received ? "Received CTRL-C" : "Timeout reached");
    return 0;
}
