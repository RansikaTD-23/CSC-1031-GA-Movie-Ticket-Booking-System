#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "movie.h"
#include "pricing.h"
#include "booking.h"
#include "display.h"

/* ---------- Safe input helpers (never crash on bad input) ---------- */

/* Reads an integer safely. Returns 1 on success, 0 on failure (and clears buffer). */
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
        printf("Invalid input. Please enter numbers only.\n");
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
    if (!promptInt("\nEnter show # (or 0 to cancel): ", &showNum)) return -1;
    if (showNum == 0) return -1;
    if (showNum < 1 || showNum > NUM_SHOWS) {
        printf("Invalid show number.\n");
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
    printf("\nSeat map for %s (%s):\n", movies[s / SHOWTIMES_PER_MOVIE].title,
           movies[s / SHOWTIMES_PER_MOVIE].times[s % SHOWTIMES_PER_MOVIE]);
    printSeatMap(&shows[s]);
}

static void handleBookSeat(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    printSeatMap(&shows[s]);

    int numSeats;
    if (!promptInt("\nHow many seats to book in this transaction? ", &numSeats)) return;
    if (numSeats < 1 || numSeats > TOTAL_SEATS) {
        printf("Invalid number of seats.\n");
        return;
    }

    char name[MAX_NAME_LEN];
    printf("Enter customer name: ");
    readLine(name, MAX_NAME_LEN);
    if (name[0] == '\0') {
        printf("Booking cancelled: name cannot be empty.\n");
        return;
    }

    int isStudent = 0, isSenior = 0;
    int category;
    printf("Discount category — 0: None  1: Student  2: Senior Citizen: ");
    if (!readInt(&category)) category = 0;
    if (category == 1) isStudent = 1;
    else if (category == 2) isSenior = 1;

    int isGroup = (numSeats >= GROUP_MIN_SEATS) ? 1 : 0;
    if (isGroup) {
        printf("Group discount (%d%% off) applies for booking %d+ seats.\n",
               GROUP_DISCOUNT_PCT, GROUP_MIN_SEATS);
    }

    int booked = 0;
    double transactionTotal = 0.0;

    for (int i = 0; i < numSeats; i++) {
        char label[16];
        printf("\nSeat %d of %d — enter seat (e.g. C7): ", i + 1, numSeats);
        readLine(label, sizeof(label));

        int row, col;
        if (!parseSeatLabel(label, &row, &col)) {
            printf("  Invalid seat format. Skipping this seat.\n");
            continue;
        }

        double price;
        if (bookSeat(&shows[s], row, col, name, isStudent, isSenior, isGroup, &price)) {
            printf("  Booked seat %c%d for %s. Price: Rs. %.2f\n",
                   'A' + row, col + 1, name, price);
            transactionTotal += price;
            booked++;
        } else {
            printf("  Seat %c%d is already booked or out of range. Skipped.\n",
                   'A' + row, col + 1);
        }
    }

    printf("\nTransaction summary: %d seat(s) booked, total Rs. %.2f\n",
           booked, transactionTotal);
}

static void handleCancelBooking(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    printSeatMap(&shows[s]);

    char label[16];
    printf("\nEnter seat to cancel (e.g. C7): ");
    readLine(label, sizeof(label));

    int row, col;
    if (!parseSeatLabel(label, &row, &col)) {
        printf("Invalid seat format.\n");
        return;
    }

    if (cancelSeat(&shows[s], row, col)) {
        printf("Booking for seat %c%d has been cancelled and the seat is now free.\n",
               'A' + row, col + 1);
    } else {
        printf("That seat is not currently booked (nothing to cancel), or is out of range.\n");
    }
}

static void handleSearchBooking(Movie movies[], Showtime shows[]) {
    int s = chooseShowtime(movies);
    if (s < 0) return;

    int mode;
    printf("Search by — 1: Customer name  2: Seat number: ");
    if (!readInt(&mode)) {
        printf("Invalid input.\n");
        return;
    }

    if (mode == 1) {
        char name[MAX_NAME_LEN];
        printf("Enter (part of) customer name: ");
        readLine(name, MAX_NAME_LEN);
        int found = searchByName(&shows[s], name);
        if (found == 0) printf("No bookings found for \"%s\".\n", name);
    } else if (mode == 2) {
        char label[16];
        printf("Enter seat number (e.g. C7): ");
        readLine(label, sizeof(label));
        int row, col;
        if (!parseSeatLabel(label, &row, &col)) {
            printf("Invalid seat format.\n");
            return;
        }
        if (!searchBySeat(&shows[s], row, col)) {
            printf("Seat %c%d is not currently booked.\n", 'A' + row, col + 1);
        }
    } else {
        printf("Invalid choice.\n");
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
            printf("Invalid input. Please enter a number between 1 and 7.\n");
            continue;
        }

        switch (choice) {
            case 1: handleViewShowtimes(movies); break;
            case 2: handleViewSeatMap(movies, shows); break;
            case 3: handleBookSeat(movies, shows); break;
            case 4: handleCancelBooking(movies, shows); break;
            case 5: handleSearchBooking(movies, shows); break;
            case 6: printRevenueReport(movies, shows); break;
            case 7:
                printf("Thank you for using the Movie Ticket Booking System. Goodbye!\n");
                running = 0;
                break;
            default:
                printf("Invalid choice. Please select an option between 1 and 7.\n");
        }
    }

    return 0;
}
