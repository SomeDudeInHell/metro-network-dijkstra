#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Struct to represent an edge in the adjacency list
typedef struct Edge {
    int destination;        // Index of the destination station
    int distance;           // Distance to the destination station
    struct Edge *next;      // Pointer to the next edge
} Edge;

// Struct to store the metro network
typedef struct {
    char *cityName;         // Pointer to city name
    int numStations;        // Number of stations
    char **stationNames;    // Pointer to array of station names
    Edge **adjList;         // Adjacency list (array of pointers to edges)
} MetroNetwork;

// Function prototypes
void dijkstra(MetroNetwork *network, int startStation, int endStation);
MetroNetwork* readMetroNetworkFromFile(const char *filename);
void freeMetroNetwork(MetroNetwork *network);
void printMetroNetwork(MetroNetwork *network);

// Dijkstra's algorithm
void dijkstra(MetroNetwork *network, int startStation, int endStation) {
    int numStations = network->numStations;
    int distances[numStations];
    int previous[numStations];
    int visited[numStations];

    // Initialize arrays
    for (int i = 0; i < numStations; i++) {
        distances[i] = INT_MAX;
        visited[i] = 0;
        previous[i] = -1;
    }
    distances[startStation] = 0;

    // Main loop for Dijkstra's algorithm
    for (int i = 0; i < numStations; i++) {
        int u = -1, minDist = INT_MAX;

        // Find the unvisited station with the smallest distance
        for (int j = 0; j < numStations; j++) {
            if (!visited[j] && distances[j] < minDist) {
                minDist = distances[j];
                u = j;
            }
        }

        if (u == -1) break;
        visited[u] = 1;

        // Relax edges in adjacency list
        Edge *current = network->adjList[u];
        while (current) {
            int v = current->destination;
            if (!visited[v] && distances[u] + current->distance < distances[v]) {
                distances[v] = distances[u] + current->distance;
                previous[v] = u;
            }
            current = current->next;
        }
    }

    // Check if the end station is reachable
    if (distances[endStation] == INT_MAX) {
        printf("No path exists between %s and %s.\n",
               network->stationNames[startStation], network->stationNames[endStation]);
        return;
    }

    // Print the shortest path in reverse order
    printf("Shortest path from %s to %s (distance: %d):\n",
           network->stationNames[startStation],
           network->stationNames[endStation],
           distances[endStation]);

    int current = endStation;
    while (current != -1) {
        printf("%s", network->stationNames[current]);
        current = previous[current];
        if (current != -1) {
            printf(" <- ");
        }
    }
    printf("\n");
}

// Print Metro Network
void printMetroNetwork(MetroNetwork *network) {
    if (!network) {
        printf("No metro network data available.\n");
        return;
    }

    printf("City: %s\n", network->cityName);
    printf("Number of Stations: %d\n", network->numStations);
    printf("Stations:\n");
    for (int i = 0; i < network->numStations; i++) {
        printf("  %d. %s\n", i + 1, network->stationNames[i]);
    }

    printf("Adjacency List:\n");
    for (int i = 0; i < network->numStations; i++) {
        printf("%s: ", network->stationNames[i]);
        Edge *current = network->adjList[i];
        while (current) {
            printf("-> (%s, %d) ", network->stationNames[current->destination], current->distance);
            current = current->next;
        }
        printf("\n");
    }
}

// Free Metro Network
void freeMetroNetwork(MetroNetwork *network) {
    if (!network) return;

    if (network->cityName) {
        free(network->cityName);
    }

    if (network->stationNames) {
        for (int i = 0; i < network->numStations; i++) {
            free(network->stationNames[i]);
        }
        free(network->stationNames);
    }

    if (network->adjList) {
        for (int i = 0; i < network->numStations; i++) {
            Edge *current = network->adjList[i];
            while (current) {
                Edge *temp = current;
                current = current->next;
                free(temp);
            }
        }
        free(network->adjList);
    }

    free(network);
}

// Read Metro Network from File
MetroNetwork* readMetroNetworkFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    MetroNetwork *network = malloc(sizeof(MetroNetwork));
    if (!network) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read city name
    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), file)) {
        perror("Error reading city name");
        free(network);
        fclose(file);
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    network->cityName = malloc(strlen(buffer) + 1);
    strcpy(network->cityName, buffer);

    // Read number of stations
    if (fscanf(file, "%d\n", &network->numStations) != 1) {
        perror("Error reading number of stations");
        freeMetroNetwork(network);
        fclose(file);
        return NULL;
    }

    // Allocate memory for station names
    network->stationNames = malloc(network->numStations * sizeof(char *));
    for (int i = 0; i < network->numStations; i++) {
        if (!fgets(buffer, sizeof(buffer), file)) {
            perror("Error reading station name");
            freeMetroNetwork(network);
            fclose(file);
            return NULL;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        network->stationNames[i] = malloc(strlen(buffer) + 1);
        strcpy(network->stationNames[i], buffer);
    }

    // Initialize adjacency list
    network->adjList = malloc(network->numStations * sizeof(Edge *));
    for (int i = 0; i < network->numStations; i++) {
        network->adjList[i] = NULL;
    }

    // Read edges
    while (fgets(buffer, sizeof(buffer), file)) {
        char src[256], dest[256];
        int distance;
        if (sscanf(buffer, "%s %s %d", src, dest, &distance) != 3) {
            perror("Error reading edges");
            freeMetroNetwork(network);
            fclose(file);
            return NULL;
        }

        // Find indices of source and destination stations
        int srcIndex = -1, destIndex = -1;
        for (int i = 0; i < network->numStations; i++) {
            if (strcmp(src, network->stationNames[i]) == 0) {
                srcIndex = i;
            }
            if (strcmp(dest, network->stationNames[i]) == 0) {
                destIndex = i;
            }
        }

        if (srcIndex == -1 || destIndex == -1) {
            fprintf(stderr, "Invalid station names in edge: %s %s\n", src, dest);
            freeMetroNetwork(network);
            fclose(file);
            return NULL;
        }

        // Add edge to adjacency list
        Edge *newEdge = malloc(sizeof(Edge));
        newEdge->destination = destIndex;
        newEdge->distance = distance;
        newEdge->next = network->adjList[srcIndex];
        network->adjList[srcIndex] = newEdge;
    }

    fclose(file);
    return network;
}

// Main function
int main(int argc, char *argv[]) {
    const char *inputFile = "metro.txt";

    MetroNetwork *network = readMetroNetworkFromFile(inputFile);
    if (!network) {
        fprintf(stderr, "Failed to load metro network.\n");
        return 1;
    }

    printMetroNetwork(network);

    char buffer[256];
    int startStation = -1, endStation = -1;

    // Get start station
    while (startStation == -1) {
        printf("Enter the start station: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        for (int i = 0; i < network->numStations; i++) {
            if (strcmp(buffer, network->stationNames[i]) == 0) {
                startStation = i;
                break;
            }
        }
        if (startStation == -1) {
            printf("Station not found. Please try again.\n");
        }
    }

    // Get end station
    while (endStation == -1) {
        printf("Enter the end station: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        for (int i = 0; i < network->numStations; i++) {
            if (strcmp(buffer, network->stationNames[i]) == 0) {
                endStation = i;
                break;
            }
        }
        if (endStation == -1) {
            printf("Station not found. Please try again.\n");
        }
    }

    // Call Dijkstra's algorithm
    dijkstra(network, startStation, endStation);

    freeMetroNetwork(network);
    return 0;
}
