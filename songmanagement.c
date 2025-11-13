#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR 100
#define SAVE_FILE "playlist_data.txt"

// --- Structures ---

typedef struct Song {
    char title[MAX_STR];
    char artist[MAX_STR];
    int duration; // in seconds
    struct Song* next;
} Song;

typedef struct Playlist {
    char name[MAX_STR];
    Song* head;
    Song* tail; // OPTIMIZATION: Keep track of end for fast insertion
    int count;  // Keep track of number of songs
} Playlist;

// --- Helper Functions ---

// Safe input function to handle spaces and prevent buffer overflow
void getLine(char* buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

// Case insensitive string comparison wrapper
int strCaseCmpr(const char* s1, const char* s2) {
    char c1, c2;
    while (*s1 && *s2) {
        c1 = tolower((unsigned char)*s1);
        c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseExec() {
    printf("\nPress Enter to continue...");
    getchar();
}

// --- Core Functions ---

void initPlaylist(Playlist* pl, const char* name) {
    strcpy(pl->name, name);
    pl->head = NULL;
    pl->tail = NULL;
    pl->count = 0;
}

Song* createSong(char* title, char* artist, int duration) {
    Song* newSong = (Song*)malloc(sizeof(Song));
    if (!newSong) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    strcpy(newSong->title, title);
    strcpy(newSong->artist, artist);
    newSong->duration = duration;
    newSong->next = NULL;
    return newSong;
}

void addSong(Playlist* pl, char* title, char* artist, int duration) {
    Song* newSong = createSong(title, artist, duration);
    
    if (pl->head == NULL) {
        pl->head = newSong;
        pl->tail = newSong;
    } else {
        pl->tail->next = newSong; // Link old tail to new song
        pl->tail = newSong;       // Update tail pointer
    }
    pl->count++;
    printf(" [Success] Added: '%s' by %s\n", title, artist);
}

void removeSong(Playlist* pl, char* title) {
    if (pl->head == NULL) {
        printf(" [Error] Playlist is empty!\n");
        return;
    }

    Song* temp = pl->head;
    Song* prev = NULL;

    // Search for the song (Case insensitive)
    while (temp != NULL && strCaseCmpr(temp->title, title) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf(" [Error] Song not found: %s\n", title);
        return;
    }

    // Unlinking the node
    if (prev == NULL) {
        // Removing the head
        pl->head = temp->next;
        if (pl->head == NULL) pl->tail = NULL; // List became empty
    } else {
        prev->next = temp->next;
        if (prev->next == NULL) pl->tail = prev; // Removed the tail
    }

    free(temp);
    pl->count--;
    printf(" [Success] Removed song: %s\n", title);
}

void searchSong(Playlist* pl, char* keyword) {
    if (pl->head == NULL) {
        printf("Playlist is empty.\n");
        return;
    }
    
    Song* temp = pl->head;
    int found = 0;
    printf("\n--- Search Results for '%s' ---\n", keyword);
    
    while (temp != NULL) {
        // Search in Title OR Artist
        if (strstr(temp->title, keyword) || strstr(temp->artist, keyword)) {
            printf(" > %s by %s (%dm %ds)\n", temp->title, temp->artist, temp->duration/60, temp->duration%60);
            found = 1;
        }
        temp = temp->next;
    }
    
    if (!found) printf("No matches found.\n");
}

void displayPlaylist(Playlist* pl) {
    if (pl->head == NULL) {
        printf("\n--- Playlist: %s (Empty) ---\n", pl->name);
        return;
    }

    printf("\n--- %s (%d Songs) ---\n", pl->name, pl->count);
    printf("%-30s %-30s %-10s\n", "Title", "Artist", "Duration");
    printf("---------------------------------------------------------------------------\n");
    
    Song* temp = pl->head;
    int totalSeconds = 0;
    
    while (temp != NULL) {
        printf("%-30s %-30s %d:%02d\n", temp->title, temp->artist, temp->duration / 60, temp->duration % 60);
        totalSeconds += temp->duration;
        temp = temp->next;
    }
    
    printf("---------------------------------------------------------------------------\n");
    printf("Total Playtime: %d min %d sec\n", totalSeconds / 60, totalSeconds % 60);
}

// --- File I/O Functions ---

void savePlaylist(Playlist* pl) {
    FILE* fp = fopen(SAVE_FILE, "w");
    if (!fp) {
        printf("Error saving playlist.\n");
        return;
    }
    
    Song* temp = pl->head;
    while (temp != NULL) {
        // Format: Title|Artist|Duration
        fprintf(fp, "%s|%s|%d\n", temp->title, temp->artist, temp->duration);
        temp = temp->next;
    }
    fclose(fp);
    printf(" [System] Playlist saved to '%s'.\n", SAVE_FILE);
}

void loadPlaylist(Playlist* pl) {
    FILE* fp = fopen(SAVE_FILE, "r");
    if (!fp) {
        printf(" [System] No saved playlist found. Starting fresh.\n");
        return;
    }
    
    char line[256];
    char title[MAX_STR], artist[MAX_STR];
    int duration;
    
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse using strtok
        char* token = strtok(line, "|");
        if (token) strcpy(title, token);
        
        token = strtok(NULL, "|");
        if (token) strcpy(artist, token);
        
        token = strtok(NULL, "|");
        if (token) duration = atoi(token);
        
        addSong(pl, title, artist, duration);
    }
    
    fclose(fp);
    // Prevent "Added" messages from cluttering startup
    clearScreen(); 
    printf(" [System] Loaded %d songs from file.\n", pl->count);
}

void freePlaylist(Playlist* pl) {
    Song* temp = pl->head;
    while (temp != NULL) {
        Song* next = temp->next;
        free(temp);
        temp = next;
    }
    pl->head = NULL;
    pl->tail = NULL;
    pl->count = 0;
}

// --- Main ---

int main() {
    Playlist myPlaylist;
    initPlaylist(&myPlaylist, "My Favorites");

    // Load data on startup
    loadPlaylist(&myPlaylist);

    int choice;
    char buffer[MAX_STR];
    char title[MAX_STR], artist[MAX_STR];
    int duration;

    while (1) {
        // clearScreen(); // Optional: Uncomment if you want a static-looking menu
        printf("\n=== SPOTIFY C-MANAGER ===\n");
        printf("1. Add Song\n");
        printf("2. Remove Song\n");
        printf("3. View Playlist\n");
        printf("4. Search Song\n");
        printf("5. Save & Exit\n");
        printf("Enter choice: ");
        
        // Robust Integer Input
        getLine(buffer, sizeof(buffer));
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                printf("Enter Title: ");
                getLine(title, sizeof(title));
                printf("Enter Artist: ");
                getLine(artist, sizeof(artist));
                printf("Enter Duration (seconds): ");
                getLine(buffer, sizeof(buffer));
                duration = atoi(buffer);
                
                if (duration > 0) {
                    addSong(&myPlaylist, title, artist, duration);
                } else {
                    printf("Invalid duration.\n");
                }
                break;
            case 2:
                displayPlaylist(&myPlaylist);
                printf("Enter Title to Remove (exact or case-insensitive): ");
                getLine(title, sizeof(title));
                removeSong(&myPlaylist, title);
                break;
            case 3:
                displayPlaylist(&myPlaylist);
                break;
            case 4:
                printf("Enter Search Term (Artist or Title): ");
                getLine(buffer, sizeof(buffer));
                searchSong(&myPlaylist, buffer);
                break;
            case 5:
                savePlaylist(&myPlaylist);
                freePlaylist(&myPlaylist);
                printf("Exiting... Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice! Try again.\n");
        }
        // pauseExec(); // Uncomment if you are using clearScreen() inside the loop
    }
    return 0;
}
