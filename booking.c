#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "booking.h"
#include "pricing.h"

void initShowtimes(Showtime shows[]) {
    int s, r, c;
    for (s = 0; s < NUM_SHOWS; s++) {
        shows[s].ticketsSold = 0;
        shows[s].totalRevenue = 0.0;
        for (r = 0; r < ROWS; r++) {
            for (c = 0; c < SEATS_PER_ROW; c++) {
                shows[s].seats[r][c].booked = 0;
                shows[s].seats[r][c].pricePaid = 0.0;
                shows[s].seats[r][c].isGroupBooking = 0;
                shows[s].seats[r][c].customerName[0] = '\0';
            }
        }
    }
}

int isValidSeat(int row, int col) {
    return (row >= 0 && row < ROWS && col >= 0 && col < SEATS_PER_ROW);
}

int bookSeat(Showtime *show, int row, int col, const char *name,
             int isStudent, int isSenior, int isGroup, double *priceOut) {
    if (!isValidSeat(row, col)) return 0;
    if (show->seats[row][col].booked) return 0;

    SeatTier tier = getSeatTier(row);
    double price = calculatePrice(tier, isStudent, isSenior, isGroup);

    show->seats[row][col].booked = 1;
    strncpy(show->seats[row][col].customerName, name, MAX_NAME_LEN - 1);
    show->seats[row][col].customerName[MAX_NAME_LEN - 1] = '\0';
    show->seats[row][col].pricePaid = price;
    show->seats[row][col].isGroupBooking = isGroup;

    show->ticketsSold++;
    show->totalRevenue += price;

    if (priceOut) *priceOut = price;
    return 1;
}

int cancelSeat(Showtime *show, int row, int col) {
    if (!isValidSeat(row, col)) return 0;
    if (!show->seats[row][col].booked) return 0;

    /* Refund: remove this seat's price from revenue and ticket count */
    show->totalRevenue -= show->seats[row][col].pricePaid;
    show->ticketsSold--;

    show->seats[row][col].booked = 0;
    show->seats[row][col].pricePaid = 0.0;
    show->seats[row][col].isGroupBooking = 0;
    show->seats[row][col].customerName[0] = '\0';

    return 1;
}

/* Case-insensitive substring check: does haystack contain needle? */
static int containsIgnoreCase(const char *haystack, const char *needle) {
    if (needle[0] == '\0') return 0;
    int hLen = (int)strlen(haystack);
    int nLen = (int)strlen(needle);
    for (int i = 0; i + nLen <= hLen; i++) {
        int match = 1;
        for (int j = 0; j < nLen; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}

/* Prints the shared column header for search results. */
static void printSearchResultHeader(void) {
    printf("  %-6s | %-20s | %12s\n", "Seat", "Customer", "Price Paid");
    printf("  ------ | -------------------- | ------------\n");
}

/* Prints one aligned search-result row. Seat label is built into a
   fixed-width field first so 1-digit and 2-digit seat numbers (A1 vs
   J10) still line up, and the price uses a fixed-width numeric field
   so amounts of any size stay in their column. */
static void printSearchResultRow(int row, int col, const char *name, double price) {
    char seatLabel[8];
    snprintf(seatLabel, sizeof(seatLabel), "%c%d", 'A' + row, col + 1);
    printf("  %-6s | %-20s | Rs. %8.2f\n", seatLabel, name, price);
}

int searchByName(const Showtime *show, const char *name) {
    int found = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < SEATS_PER_ROW; c++) {
            if (show->seats[r][c].booked &&
                containsIgnoreCase(show->seats[r][c].customerName, name)) {
                if (found == 0) printSearchResultHeader();
                printSearchResultRow(r, c, show->seats[r][c].customerName,
                                     show->seats[r][c].pricePaid);
                found++;
            }
        }
    }
    return found;
}

int searchBySeat(const Showtime *show, int row, int col) {
    if (!isValidSeat(row, col)) return 0;
    if (!show->seats[row][col].booked) return 0;

    printSearchResultHeader();
    printSearchResultRow(row, col, show->seats[row][col].customerName,
                         show->seats[row][col].pricePaid);
    return 1;
}
