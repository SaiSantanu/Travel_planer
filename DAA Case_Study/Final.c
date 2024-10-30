#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PLACES 20  // Max number of places
#define COST_PER_KM 12  // Cost per km by car
#define TIME_PER_KM 2   // Time per km in minutes (2 min per km)

// Structure to store travel information between places
typedef struct {
    char from[50];
    char to[50];
    float distance;
} TravelInfo;

// Structure to store information about places (ratings and costs)
typedef struct {
    char name[50];
    float rating;
    float visitingCost;
} PlaceInfo;

// Structure to store city distance information from CITY_LINKS.csv
typedef struct {
    char fromCity[50];
    char toCity[50];
    float distance;
} CityLink;

// Function to calculate total cost
float calculateTotalCost(TravelInfo travel[], PlaceInfo places[], int numPlaces) {
    float totalCost = 0.0;
    for (int i = 0; i < numPlaces; i++) {
        totalCost += travel[i].distance * COST_PER_KM;  // Travel cost
        totalCost += places[i].visitingCost;             // Visiting cost
    }
    return totalCost;
}

// Function to filter places by rating to fit budget
void filterPlacesByBudget(TravelInfo travel[], PlaceInfo places[], int* numPlaces, float budget) {
    float totalCost = calculateTotalCost(travel, places, *numPlaces);

    // Sort places by rating in ascending order to remove lower-rated places first
    for (int i = 1; i < *numPlaces - 1; i++) {  // Start from index 1 to skip the starting node
        for (int j = i + 1; j < *numPlaces; j++) {
            if (places[i].rating > places[j].rating) {
                // Swap places
                PlaceInfo tempPlace = places[i];
                places[i] = places[j];
                places[j] = tempPlace;

                // Swap corresponding travel distances
                TravelInfo tempTravel = travel[i];
                travel[i] = travel[j];
                travel[j] = tempTravel;
            }
        }
    }

    // Remove lower-rated places until total cost is within budget
    while (totalCost > budget && *numPlaces > 1) {
        totalCost -= (travel[*numPlaces - 1].distance * COST_PER_KM) + places[*numPlaces - 1].visitingCost;
        (*numPlaces)--;
    }
}

void displayOptimalPath(TravelInfo travel[], PlaceInfo places[], int numPlaces) {
    float totalDistance = 0.0, totalCost = 0.0, totalVisitingCost = 0.0;
    float totalTime = 0.0, totalRating = 0.0;

    bool visited[numPlaces];
    for (int i = 0; i < numPlaces; i++) {
        visited[i] = false;  // Initialize all places as unvisited
    }

    int currentPlace = 0;  // Start from the first place (index 0)
    visited[currentPlace] = true;  // Mark the starting place as visited
    printf("Starting the path calculation from %s\n", travel[currentPlace].from);

    for (int count = 0; count < numPlaces - 1; count++) {
        int nearestPlace = -1;
        float nearestDistance = 99999.0;  // A large number to find the minimum

        // Find the nearest unvisited place
        for (int j = 0; j < numPlaces; j++) {
            if (!visited[j] && travel[j].distance < nearestDistance) {
                nearestDistance = travel[j].distance;
                nearestPlace = j;
            }
        }

        // If a nearest unvisited place is found, travel to it
        if (nearestPlace != -1) {
            printf("Currently at: %s, Traveling to: %s\n", travel[currentPlace].to, travel[nearestPlace].to);
            

            // Update total values
            totalDistance += travel[nearestPlace].distance;
            totalTime += (travel[nearestPlace].distance * TIME_PER_KM) / 60.0;
            totalCost += travel[nearestPlace].distance * COST_PER_KM;
            totalVisitingCost += places[nearestPlace].visitingCost;
            totalRating += places[nearestPlace].rating;

            // Move to the next place
            currentPlace = nearestPlace;
            visited[currentPlace] = true;  // Mark the new place as visited
        }
    }

    // Return to the starting point (round trip)
    printf("\nReturning to %s\n", travel[0].from);
    totalDistance += travel[0].distance;
    totalTime += (travel[0].distance * TIME_PER_KM) / 60.0;
    totalCost += travel[0].distance * COST_PER_KM;

    // Print the summary
    printf("\nTotal travel distance: %.1f km\n", totalDistance);
    printf("Total travel time: %.2f hours (%.2f minutes)\n", totalTime, totalTime * 60);
    printf("Total travel cost (by car): %.2f Rs\n", totalCost);
    printf("Total visiting cost: %.2f Rs\n", totalVisitingCost);
    printf("Total place ratings sum: %.2f\n", totalRating);
}


// Function to suggest the next city based on CITY_LINKS.csv data
void suggestNextCity(int currentCity, CityLink cityLinks[], int numLinks) {
    const char *currentCityName;

    // Set the current city name based on user's choice
    switch (currentCity) {
        case 1: currentCityName = "Bhubaneswar"; break;
        case 2: currentCityName = "Cuttack"; break;
        case 3: currentCityName = "Puri"; break;
        default: printf("Invalid city choice\n"); return;
    }

    printf("\nYou have completed visiting your selected city.\n");

    // Find the closest city from the current city
    float closestDistance = 9999.0;
    const char *nextCity = NULL;
    for (int i = 0; i < numLinks; i++) {
        if (strcmp(cityLinks[i].fromCity, currentCityName) == 0 && cityLinks[i].distance < closestDistance) {
            closestDistance = cityLinks[i].distance;
            nextCity = cityLinks[i].toCity;
        }
    }

    if (nextCity) {
        printf("Next closest city is %s (%.1f km away).\n", nextCity, closestDistance);
    } else {
        printf("No further city links found.\n");
    }
}

// Function to read city links from CITY_LINKS.csv
int readCityLinks(const char* fileName, CityLink cityLinks[], int* numLinks) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        printf("Error: Could not open city links file: %s\n", fileName);
        return 0;
    }

    char line[1024];
    int idx = 0;
    fgets(line, sizeof(line), file);  // Skip header
    while (fgets(line, sizeof(line), file) && idx < MAX_PLACES) {
        sscanf(line, "%[^,],%[^,],%f", cityLinks[idx].fromCity, cityLinks[idx].toCity, &cityLinks[idx].distance);
        idx++;
    }
    fclose(file);

    *numLinks = idx;
    return 1;
}

// Function to read data from CSV files for travel and place information
int readCSV(const char* pathFile, const char* placeFile, TravelInfo travel[], PlaceInfo places[], int* numPlaces) {
    FILE *file = fopen(pathFile, "r");
    if (!file) {
        printf("Error: Could not open path file: %s\n", pathFile);
        return 0;
    }

    // Read the travel CSV for distances
    char line[1024];
    int idx = 0;
    fgets(line, sizeof(line), file);  // Skip header
    while (fgets(line, sizeof(line), file) && idx < MAX_PLACES) {
        sscanf(line, "%[^,],%[^,],%f", travel[idx].from, travel[idx].to, &travel[idx].distance);
        idx++;
    }
    fclose(file);

    *numPlaces = idx;  // Store the number of places

    // Read the places CSV for ratings and costs
    file = fopen(placeFile, "r");
    if (!file) {
        printf("Error: Could not open place file: %s\n", placeFile);
        return 0;
    }

    idx = 0;
    fgets(line, sizeof(line), file);  // Skip header
    while (fgets(line, sizeof(line), file) && idx < MAX_PLACES) {
        sscanf(line, "%[^,],%f,%f", places[idx].name, &places[idx].rating, &places[idx].visitingCost);
        idx++;
    }
    fclose(file);

    return 1;
}

int main() {
    TravelInfo travel[MAX_PLACES];
    PlaceInfo places[MAX_PLACES];
    CityLink cityLinks[MAX_PLACES];
    int numPlaces = 0, numLinks = 0;

    char pathFile[256], placeFile[256], cityLinksFile[] = "CITY_LINKS.csv";
    int choice;

    // Load city links from file once (outside loop since it's static data)
    if (!readCityLinks(cityLinksFile, cityLinks, &numLinks)) {
        printf("Could not load city links data.\n");
        return 1;
    }

    // Loop to keep displaying the menu until the user chooses "Exit"
    while (1) {
        printf("\n*****************************************************\n");
        printf("*      Welcome to the Odisha City Tour Planner!     *\n");
        printf("*****************************************************\n");
        printf("*    Select a city to start your adventure:         *\n");
        printf("*    1. Bhubaneswar    (The Temple City)            *\n");
        printf("*    2. Cuttack        (The Silver City)            *\n");
        printf("*    3. Puri           (The Holy Beach City)        *\n");
        printf("*    4. Exit                                        *\n");
        printf("*****************************************************\n");
        printf("Enter the number corresponding to your choice(1/2/3/4) :");
        scanf("%d", &choice);

        if (choice == 4) {
            printf("\n***********************************************************\n");
            printf("!!Exiting the tour planner. Thank you for visiting Odisha!!");
            printf("\n***********************************************************\n");
            break;  // Exit the loop if the user chooses "Exit"
        }

        // Assign city files based on choice
        switch (choice) {
            case 1:
                strcpy(pathFile, "BHUBANESWAR_PATH.csv");
                strcpy(placeFile, "BHUBANESWAR_PLACE.csv");
                break;
            case 2:
                strcpy(pathFile, "CUTTACK_PATH.csv");
                strcpy(placeFile, "CUTTACK_PLACE.csv");
                break;
            case 3:
                strcpy(pathFile, "PURI_PATH.csv");
                strcpy(placeFile, "PURI_PLACE.csv");
                break;
            default:
                printf("Invalid choice. Please select a valid city or choose Exit.\n");
                continue;  // Go back to the start of the loop
        }

        // Load place and path data from CSV files for the selected city
        if (!readCSV(pathFile, placeFile, travel, places, &numPlaces)) {
            printf("Error reading CSV files for %s.\n", (choice == 1) ? "Bhubaneswar" : (choice == 2) ? "Cuttack" : "Puri");
            continue;  // Go back to the start of the loop if loading fails
        }

        // Prompt the user for budget and calculate the optimal path
        float budget;
        printf("Enter your budget (in Rs): ");
        scanf("%f", &budget);

        filterPlacesByBudget(travel, places, &numPlaces, budget);
        displayOptimalPath(travel, places, numPlaces);

        // Suggest the next closest city based on the current selection
        suggestNextCity(choice, cityLinks, numLinks);

        // Ask if the user wants to explore another city or exit
        printf("\nWould you like to explore another city? (Enter 1 for Yes, 4 for Exit): ");
        scanf("%d", &choice);
        
        // If the user chooses "Exit," break the loop; otherwise, continue with the next city selection
        if (choice == 4) {
            printf("\n***********************************************************\n");
            printf("!!Exiting the tour planner. Thank you for visiting Odisha!!");
            printf("\n***********************************************************\n");
            break;
        }
    }

    return 0;
}