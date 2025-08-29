#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define IMAGE_SIZE 24
#define BLOCK_SIZE 3
#define NUM_BLOCKS (IMAGE_SIZE / BLOCK_SIZE)
#define NUM_PATTERNS 7
#define POP_SIZE 500
#define GENERATIONS 500
#define MUTATION_RATE 0.05
#define NUM_IMAGES 5
#define NEXT_GEN_METHOD 0

/** \brief structure representing the candidate for the best solution
 *
 * \param1: 3D array holding the patterns
 * \param2: integer representing its accuracy
 *
 */
typedef struct {
    int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE];
    int fitness;
} Candidate;



/** \brief: function to generate 7 different patterns randomly
 *
 * \param1: 3D array to hold the randomly created patterns
 * \return: nothing. void function
 *
 */
void generatePatterns(int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE]);



/** \brief: function to print the patterns on the screen
 *
 * \param1: 3D array to hold the randomly created patterns
 * \return: nothing. void function
 *
 */
void printPatterns(int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE]);



/** \brief: function to save the patterns to an outside file for modeling purposes
 *
 * \param1: file to write the patterns to
 * \param2: 3D array of the patterns to be written
 * \return: nothing. void function
 *
 */
void logPatterns(FILE *fp, int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE]);



/** \brief: function to open an image in .txt format from the disk and upload it to int array
 *
 * \param1: name of file to be opened
 * \param2: int array that will contain the file
 * \return: -1 if unsuccessful, else 0
 *
 */
int readImage(const char *filename, int image[IMAGE_SIZE][IMAGE_SIZE]);



/** \brief: function to compute the loss between an image and a candidate solution
 *
 * \param1: int array representing the image
 * \param2: 3D array representing the candidate solution patterns
 * \return: integer representing the computed loss
 *
 */
int computeImageLoss(int image[IMAGE_SIZE][IMAGE_SIZE], int candidate[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE]);



/** \brief: function to print the image represented as int array to the screen
 *
 * \param1: int array that represents the image
 * \return: nothing. void function
 *
 */
void printImage(int image[IMAGE_SIZE][IMAGE_SIZE]);



/** \brief: function to save newly created images to an external file
 *
 * \param1: file where image is going to be stored
 * \param2: int array representing the image to be stored
 * \return: nothing. void function
 *
 */
void logImage(FILE *fp, int image[IMAGE_SIZE][IMAGE_SIZE]);



/** \brief: function to reconstruct an image using a candidate solution
 *
 * \param1: int array representing the original image
 * \param2: 3D array representing the candidate solution patterns
 * \param3: int array to store the reconstructed image
 * \return: nothing. void function
 *
 */
void reconstructImage(int image[IMAGE_SIZE][IMAGE_SIZE], int candidate[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE], int result[IMAGE_SIZE][IMAGE_SIZE]);



/** \brief: function to initialize the population with random candidate solutions
 *
 * \param1: array of Candidate structures representing the population
 * \param2: integer representing the population size
 * \return: nothing. void function
 *
 */
void initializePopulation(Candidate population[], int popSize);



/** \brief: function to evaluate the fitness of each candidate in the population
 *
 * \param1: array of Candidate structures representing the population
 * \param2: integer representing the population size
 * \param3: 3D array containing multiple images for evaluation
 * \return: nothing. void function
 *
 */
void evaluatePopulation(Candidate population[], int popSize, int images[NUM_IMAGES][IMAGE_SIZE][IMAGE_SIZE]);



/** \brief: function to select a candidate from the population using tournament selection
 *
 * \param1: array of Candidate structures representing the population
 * \param2: integer representing the population size
 * \return: selected Candidate structure
 *
 */
Candidate tournamentSelection(Candidate population[], int popSize);



/** \brief: function to perform crossover operation between two parent candidates to create a child
 *
 * \param1: pointer to the Candidate structure representing the child
 * \param2: Candidate structure representing the first parent
 * \param3: Candidate structure representing the second parent
 * \return: nothing. void function
 *
 */
void crossover(Candidate *child, Candidate parent1, Candidate parent2);



/** \brief: function to apply mutation to a candidate solution
 *
 * \param1: pointer to the Candidate structure to mutate
 * \return: nothing. void function
 *
 */
void mutate(Candidate *candidate);


int main()
{
    srand(time(NULL)); // seed used for creating random patterns
    int imgId, gen, i;
    int images[NUM_IMAGES][IMAGE_SIZE][IMAGE_SIZE]; // 3D array to hold the patterns
    char fileName[100];

    // read 5 images with error-checking.
    for(imgId=0; imgId<NUM_IMAGES; imgId++){
        printf("Enter name of image file %d: ", imgId + 1);
        scanf("%s", fileName);
        while(readImage(fileName, images[imgId]) != 0){
            printf("Error reading image file %s. Please re-enter: ", fileName);
            scanf("%s", fileName);
        }
    }

    // show the 5 original images.
    for(imgId=0; imgId<NUM_IMAGES; imgId++){
        printf("\nOriginal Image %d (bit notation):\n", imgId + 1);
        printImage(images[imgId]);
    }
    printf("\n");

    // open two files for logging.
    FILE *fp_csv = fopen("results_ps10_m02_b3.csv", "w");
    if(fp_csv == NULL){
        printf("Error opening results_ps10_m02_b3.csv for writing.\n");
        return -1;
    }
    FILE *fp_details = fopen("detailed_results_ps10_m02_b3.txt", "w");
    if(fp_details == NULL){
        printf("Error opening detailed_results_ps10_m02_b3.txt for writing.\n");
        return -1;
    }

    // write CSV header.
    fprintf(fp_csv, "Generation,BestLoss,AverageLoss\n");

    // GA population initialization.
    Candidate population[POP_SIZE];
    initializePopulation(population, POP_SIZE);

    // log and show initial patterns (from the first candidate) before evolution.
    fprintf(fp_details, "Initial Random Patterns (from first candidate):\n");
    logPatterns(fp_details, population[0].patterns);
    fprintf(fp_details, "\n");

    // Evaluate initial population on all images.
    evaluatePopulation(population, POP_SIZE, images);

    // GA main loop.
    for(gen=0; gen<GENERATIONS; gen++){
        Candidate newPopulation[POP_SIZE];

        if(NEXT_GEN_METHOD == 0){
            // From scratch: generate POP_SIZE new individuals.
            for(i=0; i<POP_SIZE; i++){
                Candidate parent1 = tournamentSelection(population, POP_SIZE);
                Candidate parent2 = tournamentSelection(population, POP_SIZE);
                Candidate child;
                crossover(&child, parent1, parent2);
                mutate(&child);
                newPopulation[i] = child;
            }
        }
        else{
            // Substitution: use elitism (retain best candidate) and generate the rest.
            Candidate best = population[0];
            for(i=1; i<POP_SIZE; i++){
                if(population[i].fitness < best.fitness)
                    best = population[i];
            }
            newPopulation[0] = best; // Keep the elite candidate.
            for(i = 1; i < POP_SIZE; i++){
                Candidate parent1 = tournamentSelection(population, POP_SIZE);
                Candidate parent2 = tournamentSelection(population, POP_SIZE);
                Candidate child;
                crossover(&child, parent1, parent2);
                mutate(&child);
                newPopulation[i] = child;
            }
        }
        // Replace old population with new and re-evaluate fitness on all images.
        memcpy(population, newPopulation, sizeof(newPopulation));
        evaluatePopulation(population, POP_SIZE, images);

        // Compute best and average fitness for this generation.
        int bestFitness = population[0].fitness;
        int totalFitness = 0;
        for(i=0; i<POP_SIZE; i++){
            totalFitness += population[i].fitness;
            if (population[i].fitness < bestFitness)
                bestFitness = population[i].fitness;
        }
        int averageFitness = totalFitness / POP_SIZE;

        printf("Generation %d: Best Total Loss = %d, Average Total Loss = %d\n", gen, bestFitness, averageFitness);
        // Log to CSV.
        fprintf(fp_csv, "%d,%d,%d\n", gen, bestFitness, averageFitness);
        // Log detailed info.
        fprintf(fp_details, "Generation %d: Best Total Loss = %d, Average Total Loss = %d\n", gen, bestFitness, averageFitness);
    }
    fclose(fp_csv);  // Close the CSV file.

    // Find the best candidate after evolution.
    Candidate best = population[0];
    for(i=1; i<POP_SIZE; i++){
        if (population[i].fitness < best.fitness)
            best = population[i];
    }

    fprintf(fp_details, "\nFinal Best Candidate's Patterns After %d Generations:\n", GENERATIONS);
    logPatterns(fp_details, best.patterns);
    fprintf(fp_details, "\n");

    // For each image, reconstruct the image and log both the original and reconstructed images.
    for(imgId=0; imgId<NUM_IMAGES; imgId++){
        int reconstructed[IMAGE_SIZE][IMAGE_SIZE];
        reconstructImage(images[imgId], best.patterns, reconstructed);
        int imageLoss = computeImageLoss(images[imgId], best.patterns);
        fprintf(fp_details, "Image %d:\n", imgId + 1);
        fprintf(fp_details, "Original Image:\n");
        logImage(fp_details, images[imgId]);
        fprintf(fp_details, "\nReconstructed Image:\n");
        logImage(fp_details, reconstructed);
        fprintf(fp_details, "\nLoss for Image %d = %d\n\n", imgId + 1, imageLoss);

        // Also show on console.
        printf("Reconstructed Image %d Using Evolved Patterns:\n", imgId + 1);
        printImage(reconstructed);
        printf("\n");
    }

    fclose(fp_details); // Close the detailed log file.

    return 0;
}


void generatePatterns(int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE])
{
    int i, j, k;
    for(i=0; i<NUM_PATTERNS; i++){
        for(j=0; j<BLOCK_SIZE; j++){
            for(k=0; k<BLOCK_SIZE;k++){
                patterns[i][j][k] = rand() % 2; // generate either 0 or 1
            }
        }
    }
}


void printPatterns(int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE])
{
    int i, j, k;
    for(i=0; i<NUM_PATTERNS; i++){
        printf("Pattern %d:\n", i + 1);
        for(j=0; j<BLOCK_SIZE; j++){
            for(k=0; k<BLOCK_SIZE; k++){
                printf("%2d ", patterns[i][j][k]); // print the patterns on the screen
            }
            printf("\n");
        }
        printf("\n");
    }
}


void logPatterns(FILE *fp, int patterns[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE])
{
    int i, j, k;
    for(i=0; i<NUM_PATTERNS; i++){
        fprintf(fp, "Pattern %d:\n", i + 1);
        for(j=0; j<BLOCK_SIZE; j++){
            for(k=0; k<BLOCK_SIZE; k++){
                fprintf(fp, "%d ", patterns[i][j][k]); // write the patterns to the file opened
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }
}


int readImage(const char *filename, int image[IMAGE_SIZE][IMAGE_SIZE])
{
    int i, j;
    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        printf("File opening ERROR at line: %d\n", __LINE__);
        return -1;
    }
    for(i=0; i<IMAGE_SIZE; i++){
        for(j=0; j<IMAGE_SIZE; j++){
            if(fscanf(fp, "%d", &image[i][j]) != 1){ // open the image and write it to a 2D array
                printf("Error reading pixel at (%d, %d)\n", i, j);
                fclose(fp);
                return -1;
            }
        }
    }
    fclose(fp);
    return 0;
}


int computeImageLoss(int image[IMAGE_SIZE][IMAGE_SIZE], int candidate[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE])
{
    int totalLoss = 0;
    int block[BLOCK_SIZE][BLOCK_SIZE];
    int bi, bj, i, j, p;
    int minDist, distance;
    for(bi=0; bi<NUM_BLOCKS; bi++){ // run through the images
        for(bj=0; bj<NUM_BLOCKS; bj++){
            for(i=0; i<BLOCK_SIZE; i++){ // run through the candidate pattern
                for(j=0; j<BLOCK_SIZE; j++){
                    block[i][j] = image[bi * BLOCK_SIZE + i][bj * BLOCK_SIZE + j]; // compile the value of the new block
                }
            }
            minDist = BLOCK_SIZE * BLOCK_SIZE + 1; // the min distance can be at most 9
            for(p=0; p<NUM_PATTERNS; p++){ // run through each pattern
                distance = 0;
                for(i=0; i<BLOCK_SIZE; i++){
                    for(j=0; j<BLOCK_SIZE; j++){
                        if(block[i][j] != candidate[p][i][j]) // if they differ increase the distance
                            distance++;
                    }
                }
                if(distance < minDist) // if we found a better new candidate, update min distance
                    minDist = distance;
            }
            totalLoss += minDist; // add the newly min distance to the total loss of this candidate solution
        }
    }
    return totalLoss;
}

void printImage(int image[IMAGE_SIZE][IMAGE_SIZE])
{
    int i, j;
    for(i=0; i<IMAGE_SIZE; i++){
        for(j=0; j<IMAGE_SIZE; j++){
            printf("%2d", image[i][j]); // print the images on the screen
        }
        printf("\n");
    }
}

void logImage(FILE *fp, int image[IMAGE_SIZE][IMAGE_SIZE])
{
    int i, j;
    for(i=0; i<IMAGE_SIZE; i++){
        for(j=0; j<IMAGE_SIZE; j++){
            fprintf(fp, "%2d", image[i][j]); // write the images into a file
        }
        fprintf(fp, "\n");
    }
}


void reconstructImage(int image[IMAGE_SIZE][IMAGE_SIZE], int candidate[NUM_PATTERNS][BLOCK_SIZE][BLOCK_SIZE], int result[IMAGE_SIZE][IMAGE_SIZE])
{
    int bi, bj, i, j, p;
    int bestIndex;
    int distance, minDist;
    int block[BLOCK_SIZE][BLOCK_SIZE];
    for(bi=0; bi<NUM_BLOCKS; bi++){ // run through all the blocks
        for(bj=0; bj<NUM_BLOCKS; bj++){
            for(i=0; i<BLOCK_SIZE; i++){ // run through all of the patterns
                for(j=0; j<BLOCK_SIZE; j++){
                    block[i][j] = image[bi * BLOCK_SIZE + i][bj * BLOCK_SIZE + j];
                }
            }
            bestIndex = 0;
            minDist = BLOCK_SIZE * BLOCK_SIZE + 1; // same functionality as in computeImageLoss function
            for(p=0; p<NUM_PATTERNS; p++){
                distance = 0;
                for(i=0; i<BLOCK_SIZE; i++){
                    for(j=0; j<BLOCK_SIZE; j++){
                        if(block[i][j] != candidate[p][i][j])
                            distance++;
                    }
                }
                if(distance < minDist){
                    minDist = distance;
                    bestIndex = p;
                }
            }
            for(i=0; i<BLOCK_SIZE; i++){
                for(j=0; j<BLOCK_SIZE; j++){ // find the best result possible among the best candidates
                    result[bi * BLOCK_SIZE + i][bj * BLOCK_SIZE + j] = candidate[bestIndex][i][j];
                }
            }
        }
    }
}

void initializePopulation(Candidate population[], int popSize)
{
    int i;
    for(i=0; i<popSize; i++){
        generatePatterns(population[i].patterns); // initialize each populations' fitness to 0
        population[i].fitness = 0;
    }
}


void evaluatePopulation(Candidate population[], int popSize, int images[NUM_IMAGES][IMAGE_SIZE][IMAGE_SIZE])
{
    int i, img;
    int totalLoss;
    for(i=0; i<popSize; i++){
        totalLoss = 0;
        for(img=0; img<NUM_IMAGES; img++){
            totalLoss += computeImageLoss(images[img], population[i].patterns);
        }
        population[i].fitness = totalLoss; // assign the total loss of each candidate solution to this population's fitness
    }
}

Candidate tournamentSelection(Candidate population[], int popSize)
{
    int i, index;
    Candidate best;
    best.fitness = 1e9;
    for(i=0; i<3; i++){
        index = rand() % popSize;
        if(population[index].fitness < best.fitness) // function to get the best candidate among the population
            best = population[index];
    }
    return best;
}

void crossover(Candidate *child, Candidate parent1, Candidate parent2)
{
    int i, j, p;
    for(p=0; p<NUM_PATTERNS; p++){
        Candidate source = (rand() % 2 == 0) ? parent1 : parent2; // selects traits from a random parent
        for(i=0; i<BLOCK_SIZE; i++){
            for(j=0; j<BLOCK_SIZE; j++){
                child->patterns[p][i][j] = source.patterns[p][i][j]; // creates a new candidate block from the parent
            }
        }
    }
}

void mutate(Candidate *candidate)
{
    int i, j, p;
    for(p=0; p<NUM_PATTERNS; p++){
        for(i=0; i<BLOCK_SIZE; i++){
            for(j=0; j<BLOCK_SIZE; j++){
                if((double)rand() / RAND_MAX < MUTATION_RATE)
                    candidate->patterns[p][i][j] = 1 - candidate->patterns[p][i][j]; // mutates pattern of candidate randomly
            }
        }
    }
}
