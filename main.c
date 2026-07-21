#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "movie.h"
#include "pricing.h"
#include "booking.h"
#include "display.h"


static int readInt(int *out) {
    char line[64];
    if (!fgets(line, sizeof(line), stdin)) {
        return 0;
    }
    /* strip trailing newline */
    line[strcspn(line, "\n")] = '\0';

    /* validate: optional leading +/- then digits only, non-empty */
    int i = 0;
    if (line[i] == '+' || line[i] == '-') i++;
    if (line[i] == '\0') return 0;
    for (; line[i] != '\0'; i++) {
        if (!isdigit((unsigned char)line[i])) return 0;
    }
    *out = atoi(line);
    return 1;
}

static int promptInt(const char *prompt, int *out) {
    printf("%s", prompt);
    if (!readInt(out)) {
        printf("\nInvalid input. Please Enter Numbers Only.\n");
        return 0;
    }
    return 1;
}

static void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

/* Waits for the user to press Enter before returning to the main menu */
static void pauseForMenu(void) {
    printf("\nPress Enter to Return to Main Menu");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard any leftover characters until newline */
    }
}

/* Parses a seat label like "C7" into row/col (0-indexed). Returns 1 on success. */
static int parseSeatLabel(const char *label, int *row, int *col) {
    if (!label || strlen(label) < 2) return 0;
    char rowChar = toupper((unsigned char)label[0]);
    if (rowChar < 'A' || rowChar >= 'A' + ROWS) return 0;

    const char *numPart = label + 1;
    for (int i = 0; numPart[i] != '\0'; i++) {
        if (!isdigit((unsigned char)numPart[i])) return 0;
    }
    if (numPart[0] == '\0') return 0;

    int seatNum = atoi(numPart);
    if (seatNum < 1 || seatNum > SEATS_PER_ROW) return 0;

    *row = rowChar - 'A';
    *col = seatNum - 1;
    return 1;
}

/* ---------- Movie / showtime selection ---------- */

/* Step 1: pick a movie. Returns 0-based movie index, or -1 to cancel. */
static int selectMovie(Movie movies[]) {
    printf("\n+------+----------------------------------+\n");
    printf("| %-4s | %-32s |\n", "Opt.", "Movie");
    printf("+------+----------------------------------+\n");
    for (int m = 0; m < NUM_MOVIES; m++) {
        printf("| %-4d | %-32s |\n", m + 1, movies[m].title);
    }
    printf("+------+----------------------------------+\n");
    printf("| %-4s | %-32s |\n", "0", "Cancel");
    printf("+------+----------------------------------+\n");

    int choice;
    if (!promptInt("\nPlease Enter Movie #: ", &choice)) return -1;
    if (choice == 0) return -1;
    if (choice < 1 || choice > NUM_MOVIES) {
        printf("\nInvalid Movie Number.\n");
        return -1;
    }
    return choice - 1;
}

/* Step 2: pick a showtime for the given movie. Returns 0-based local
   showtime index (within that movie), or -1 to cancel. */
static int selectShowtimeForMovie(Movie movies[], int movieIndex) {
    printf("\n+------+--------------------------------------+\n");
    printf("| Showtimes: %-32s |\n", movies[movieIndex].title);
    printf("+------+--------------------------------------+\n");
    printf("| %-4s | %-36s |\n", "Opt.", "Showtime");
    printf("+------+--------------------------------------+\n");
    for (int t = 0; t < SHOWTIMES_PER_MOVIE; t++) {
        printf("| %-4d | %-36s |\n", t + 1, movies[movieIndex].times[t]);
    }
    printf("+------+--------------------------------------+\n");
    printf("| %-4s | %-36s |\n", "0", "Cancel");
    printf("+------+--------------------------------------+\n");

    int choice;
    if (!promptInt("\nPlease Enter Showtime #: ", &choice)) return -1;
    if (choice == 0) return -1;
    if (choice < 1 || choice > SHOWTIMES_PER_MOVIE) {
        printf("\nInvalid Showtime Number.\n");
        return -1;
    }
    return choice - 1;
}

/* Combines both steps into a single global show index (0-based), or -1
   if the user cancels at either step. */
static int chooseShowtime(Movie movies[]) {
    int movieIndex = selectMovie(movies);
    if (movieIndex < 0) return -1;

    int timeIndex = selectShowtimeForMovie(movies, movieIndex);
    if (timeIndex < 0) return -1;

    return movieIndex * SHOWTIMES_PER_MOVIE + timeIndex;
}

/* Small header shown above every seat map so it's always clear which
   movie and showtime is being displayed. */
static void printSeatMapHeader(Movie movies[], int s) {
    printf("\nSeat Map For %s (%s):\n",
           movies[s / SHOWTIMES_PER_MOVIE].title,
           movies[s / SHOWTIMES_PER_MOVIE].times[s % SHOWTIMES_PER_MOVIE]);
}

/* ---------- Menu handlers ---------- */

static void handleViewShowtimes(Movie movies[]) {
    printShowtimesList(movies);
}

static void handleViewSeatMap(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;
    printSeatMapHeader(movies, s);
    printSeatMap(&shows[s]);
}

static void handleBookSeat(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    printSeatMapHeader(movies, s);
    printSeatMap(&shows[s]);

    int numSeats;
    if (!promptInt("\nHow Many Seats to Book in This Transaction? ", &numSeats)) return;
    if (numSeats < 1 || numSeats > TOTAL_SEATS) {
        printf("\nInvalid number of seats.\n");
        return;
    }

    char name[MAX_NAME_LEN];
    printf("\nPlease Enter Customer Name: ");
    readLine(name, MAX_NAME_LEN);
    if (name[0] == '\0') {
        printf("\nBooking Cancelled: Name Cannot be Empty.\n");
        return;
    }

    int isStudent = 0, isSenior = 0;
    int category;
    printf("\n+-----+------------------+\n");
    printf("| %-3s | %-16s |\n", "Opt", "Category");
    printf("+-----+------------------+\n");
    printf("| %-3s | %-16s |\n", "0", "None");
    printf("| %-3s | %-16s |\n", "1", "Student");
    printf("| %-3s | %-16s |\n", "2", "Senior Citizen");
    printf("+-----+------------------+\n");
    if (!promptInt("\nPlease Enter Category: ", &category)) category = 0;
    if (category == 1) isStudent = 1;
    else if (category == 2) isSenior = 1;

    int isGroup = (numSeats >= GROUP_MIN_SEATS) ? 1 : 0;
    if (isGroup) {
        printf("\nGroup discount (%d%% off) applies for booking %d+ seats.\n",
               GROUP_DISCOUNT_PCT, GROUP_MIN_SEATS);
    }

    int booked = 0;
    double transactionTotal = 0.0;

    char bookedLabel[TOTAL_SEATS][12];
    double bookedPrice[TOTAL_SEATS];
    int skippedCount = 0;
    char skippedMsg[TOTAL_SEATS][64];

    for (int i = 0; i < numSeats; i++) {
        char label[16];
        printf("\n  Seat %d of %d -> Enter seat (e.g. C7): ", i + 1, numSeats);
        readLine(label, sizeof(label));

        int row, col;
        if (!parseSeatLabel(label, &row, &col)) {
            printf("  -> Invalid seat format. Skipping this seat.\n");
            if (skippedCount < TOTAL_SEATS) {
                snprintf(skippedMsg[skippedCount], sizeof(skippedMsg[skippedCount]),
                         "%-8s invalid format", label);
                skippedCount++;
            }
            continue;
        }

        double price;
        if (bookSeat(&shows[s], row, col, name, isStudent, isSenior, isGroup, &price)) {
            printf("  -> Seat %c%d booked. Price: Rs. %.2f\n", 'A' + row, col + 1, price);
            if (booked < TOTAL_SEATS) {
                snprintf(bookedLabel[booked], sizeof(bookedLabel[booked]), "%c%d", 'A' + row, col + 1);
                bookedPrice[booked] = price;
            }
            transactionTotal += price;
            booked++;
        } else {
            /* parseSeatLabel already validated the row/col range, so the
               only reason bookSeat can fail here is that the seat is
               already taken - show who holds it instead of a vague message. */
            char seatLbl[8];
            snprintf(seatLbl, sizeof(seatLbl), "%c%d", 'A' + row, col + 1);
            const char *heldBy = shows[s].seats[row][col].customerName;
            const char *heldByDisplay = (heldBy[0] != '\0') ? heldBy : "Another customer";

            printf("\n  ---------------------------------------------\n");
            printf("   %-10s: %s\n", "Seat", seatLbl);
            printf("   %-10s: %s\n", "Status", "Seat Unavailable");
            printf("   %-10s: %s\n", "Held By", heldByDisplay);
            printf("  ---------------------------------------------\n");

            if (skippedCount < TOTAL_SEATS) {
                snprintf(skippedMsg[skippedCount], sizeof(skippedMsg[skippedCount]),
                         "%-4s already booked by %.20s", seatLbl, heldByDisplay);
                skippedCount++;
            }
        }
    }

    const char *categoryLabel = isSenior ? "Senior Citizen" : (isStudent ? "Student" : "None");

    printf("\n===================================================\n");
    printf("                 BOOKING RECEIPT\n");
    printf("===================================================\n");
    printf("Movie      : %s\n", movies[s / SHOWTIMES_PER_MOVIE].title);
    printf("Showtime   : %s\n", movies[s / SHOWTIMES_PER_MOVIE].times[s % SHOWTIMES_PER_MOVIE]);
    printf("Customer   : %s\n", name);
    printf("Category   : %s\n", categoryLabel);
    if (isGroup) {
        printf("Discount   : Group discount (%d%% off)\n", GROUP_DISCOUNT_PCT);
    }
    printf("---------------------------------------------------\n");
    printf("%-10s %s\n", "Seat", "Price (Rs.)");
    printf("---------------------------------------------------\n");
    for (int i = 0; i < booked; i++) {
        printf("%-10s %10.2f\n", bookedLabel[i], bookedPrice[i]);
    }
    if (booked == 0) {
        printf("(no seats were booked)\n");
    }
    printf("---------------------------------------------------\n");
    printf("Seats Booked : %d\n", booked);
    printf("Total Amount : Rs. %.2f\n", transactionTotal);
    printf("===================================================\n");

    if (skippedCount > 0) {
        printf("\nNote - the following entries were skipped:\n");
        for (int i = 0; i < skippedCount; i++) {
            printf("  - %s\n", skippedMsg[i]);
        }
    }

    if (booked > 0) {
        printf("\nThank you for booking with us!\n");
    }
}

static void handleCancelBooking(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    printSeatMapHeader(movies, s);
    printSeatMap(&shows[s]);

    char label[16];
    printf("\nPlease Enter Seat to Cancel (e.g. C7): ");
    readLine(label, sizeof(label));

    int row, col;
    if (!parseSeatLabel(label, &row, &col)) {
        printf("\nInvalid Seat Format.\n");
        return;
    }

    char seatLabel[12];
    snprintf(seatLabel, sizeof(seatLabel), "%c%d", 'A' + row, col + 1);

    int success = cancelSeat(&shows[s], row, col);

    printf("\n===================================================\n");
    printf("%s\n", success ? "            CANCELLATION CONFIRMATION" : "              CANCELLATION FAILED");
    printf("===================================================\n");
    printf("Movie      : %s\n", movies[s / SHOWTIMES_PER_MOVIE].title);
    printf("Showtime   : %s\n", movies[s / SHOWTIMES_PER_MOVIE].times[s % SHOWTIMES_PER_MOVIE]);
    printf("Seat       : %s\n", seatLabel);
    printf("Status     : %s\n", success
           ? "Cancelled - seat is now available"
           : "Not currently booked, or out of range");
    printf("===================================================\n");
}

static void handleSearchBooking(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    int mode;
    printf("\n+-----+------------------+\n");
    printf("| %-3s | %-16s |\n", "Opt", "Search By");
    printf("+-----+------------------+\n");
    printf("| %-3s | %-16s |\n", "1", "Customer Name");
    printf("| %-3s | %-16s |\n", "2", "Seat Number");
    printf("+-----+------------------+\n");
    if (!promptInt("\nPlease Select Choice: ", &mode)) {
        return;
    }

    printf("\n----------------------- Search Results ------------------------\n");

    if (mode == 1) {
        char name[MAX_NAME_LEN];
        printf("Please Enter Customer Name: ");
        readLine(name, MAX_NAME_LEN);
        int found = searchByName(&shows[s], name);
        if (found == 0) printf("No bookings found for \"%s\".\n", name);
    } else if (mode == 2) {
        char label[16];
        printf("Please Enter Seat Number (e.g. C7): ");
        readLine(label, sizeof(label));
        int row, col;
        if (!parseSeatLabel(label, &row, &col)) {
            printf("Invalid seat format.\n");
            printf("---------------------------------------------------------------\n");
            return;
        }
        if (!searchBySeat(&shows[s], row, col)) {
            printf("Seat %c%d is not currently booked.\n", 'A' + row, col + 1);
        }
    } else {
        printf("Invalid choice.\n");
    }

    printf("---------------------------------------------------------------\n");
}

int main(void) {
    Movie movies[NUM_MOVIES] = {
        {"Avengers:Doomsday",   {"10:00 AM", "6:00 PM"}},
        {"Dune 3",       {"1:00 PM",  "8:30 PM"}},
        {"Oddessy",      {"3:30 PM",  "9:45 PM"}}
    };

    static Showtime shows[NUM_SHOWS];
    initShowtimes(shows);

    int choice;
    int running = 1;

    printf("Welcome to the Movie Ticket Booking System!\n");

    while (running) {
        printMainMenu();

        if (!readInt(&choice)) {
            printf("Invalid input. Please Enter a Number Between 1 and 7.\n");
            continue;
        }

        switch (choice) {
            case 1: handleViewShowtimes(movies); pauseForMenu(); break;
            case 2: handleViewSeatMap(movies, shows); pauseForMenu(); break;
            case 3: handleBookSeat(movies, shows); pauseForMenu(); break;
            case 4: handleCancelBooking(movies, shows); pauseForMenu(); break;
            case 5: handleSearchBooking(movies, shows); pauseForMenu(); break;
            case 6: printRevenueReport(movies, shows); pauseForMenu(); break;
            case 7:
                printf("Thank You for Using the Movie Ticket Booking System. Goodbye!\n");
                running = 0;
                break;
            default:
                printf("Invalid choice. Please select an option between 1 and 7.\n");
        }
    }

    return 0;
}
