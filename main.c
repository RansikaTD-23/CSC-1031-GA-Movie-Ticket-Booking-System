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

/* ---------- Menu handlers ---------- */

static int chooseShowtime(Movie movies[]) {
    printShowtimesList(movies);
    int showNum;
    if (!promptInt("\nPlease Enter Show # (or 0 to cancel): \n", &showNum)) return -1;
    if (showNum == 0) return -1;
    if (showNum < 1 || showNum > NUM_SHOWS) {
        printf("\nInvalid Show Number.\n");
        return -1;
    }
    return showNum - 1; /* 0-indexed */
}

static void handleViewShowtimes(Movie movies[]) {
    printShowtimesList(movies);
}

static void handleViewSeatMap(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;
    printf("\nSeat Map For %s (%s):\n", movies[s / SHOWTIMES_PER_MOVIE].title,
           movies[s / SHOWTIMES_PER_MOVIE].times[s % SHOWTIMES_PER_MOVIE]);
    printSeatMap(&shows[s]);
}

static void handleBookSeat(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

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
    printf("\nDiscount Category \n0: None  \n1: Student  \n2: Senior Citizen\n\nPlease Enter Category: ");
    if (!readInt(&category)) category = 0;
    if (category == 1) isStudent = 1;
    else if (category == 2) isSenior = 1;

    int isGroup = (numSeats >= GROUP_MIN_SEATS) ? 1 : 0;
    if (isGroup) {
        printf("\nGroup discount (%d%% off) applies for booking %d+ seats.\n",
               GROUP_DISCOUNT_PCT, GROUP_MIN_SEATS);
    }

    int booked = 0;
    double transactionTotal = 0.0;

    for (int i = 0; i < numSeats; i++) {
        char label[16];
        printf("\nSeat %d of %d Enter Seat (e.g. C7): ", i + 1, numSeats);
        readLine(label, sizeof(label));

        int row, col;
        if (!parseSeatLabel(label, &row, &col)) {
            printf("\nInvalid Seat Format. Skipping This Seat.\n");
            continue;
        }

        double price;
        if (bookSeat(&shows[s], row, col, name, isStudent, isSenior, isGroup, &price)) {
            printf("\nBooked Seat %c%d for %s. Price: Rs. %.2f\n",
                   'A' + row, col + 1, name, price);
            transactionTotal += price;
            booked++;
        } else {
            printf("\nSeat %c%d is Already Booked or Out of Range. Skipped.\n",
                   'A' + row, col + 1);
        }
    }

    printf("\nTransaction Summary: \n%d Seat Booked\ntotal Rs. %.2f\n",
           booked, transactionTotal);
}

static void handleCancelBooking(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    printSeatMap(&shows[s]);

    char label[16];
    printf("\nPlease Enter Seat to Cancel (e.g. C7): ");
    readLine(label, sizeof(label));

    int row, col;
    if (!parseSeatLabel(label, &row, &col)) {
        printf("\nInvalid Seat Format.\n");
        return;
    }

    if (cancelSeat(&shows[s], row, col)) {
        printf("\nBooking for Seat %c%d Has Been Cancelled and the Seat is Now Free.\n",
               'A' + row, col + 1);
    } else {
        printf("\nThat Seat is Not Currently Booked (nothing to cancel), or is Out of Range.\n");
    }
}

static void handleSearchBooking(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    int mode;
    printf("\nSearch by \n\n1: Customer name  \n2: Seat number\n\nPlease Select Choice: ");
    if (!readInt(&mode)) {
        printf("Invalid input.\n");
        return;
    }

    if (mode == 1) {
        char name[MAX_NAME_LEN];
        printf("\nPlease Enter Customer Name: ");
        readLine(name, MAX_NAME_LEN);
        int found = searchByName(&shows[s], name);
        if (found == 0) printf("No Bookings Found For \"%s\".\n", name);
    } else if (mode == 2) {
        char label[16];
        printf("Please Enter Seat Number (e.g. C7): ");
        readLine(label, sizeof(label));
        int row, col;
        if (!parseSeatLabel(label, &row, &col)) {
            printf("Invalid Seat Format.\n");
            return;
        }
        if (!searchBySeat(&shows[s], row, col)) {
            printf("Seat %c%d is Not Currently Booked.\n", 'A' + row, col + 1);
        }
    } else {
        printf("Invalid Choice.\n");
    }
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
