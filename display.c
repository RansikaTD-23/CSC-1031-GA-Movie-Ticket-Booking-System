#include <stdio.h>
#include "display.h"
#include "pricing.h"
void printMainMenu(void) {
    printf("\n==================================================\n");
    printf("        MOVIE TICKET BOOKING SYSTEM - MAIN MENU\n");
    printf("==================================================\n");
    printf("1. View Showtimes\n");
    printf("2. View Seat Map\n");
    printf("3. Book a Seat\n");
    printf("4. Cancel a Booking\n");
    printf("5. Search Booking\n");
    printf("6. View Revenue Report\n");
    printf("7. Exit\n");
    printf("--------------------------------------------------\n");
    printf("Please Enter Your Choice: ");
}
void printShowtimesList(Movie movies[]) {
    printf("\n===================================================================\n");
    printf("                    AVAILABLE MOVIES & SHOWTIMES\n");
    printf("===================================================================\n");
    printf("+--------+----------------------------------+------------+\n");
    printf("| %-6s | %-32s | %-10s |\n", "Show #", "Movie", "Showtime");
    printf("+--------+----------------------------------+------------+\n");
    for (int m = 0; m < NUM_MOVIES; m++) {
        for (int t = 0; t < SHOWTIMES_PER_MOVIE; t++) {
            int showIndex = m * SHOWTIMES_PER_MOVIE + t + 1;
            printf("| %-6d | %-32s | %-10s |\n",
                   showIndex, movies[m].title, movies[m].times[t]);
        }
    }
    printf("+--------+----------------------------------+------------+\n");
}
void printSeatMap(const Showtime *show) {
    printf("\n      ");
    for (int c = 0; c < SEATS_PER_ROW; c++) {
        printf("%2d ", c + 1);
    }
    printf("\n");
    for (int r = 0; r < ROWS; r++) {
        printf("Row %c ", 'A' + r);
        for (int c = 0; c < SEATS_PER_ROW; c++) {
            char symbol = show->seats[r][c].booked ? 'X' : '.';
            printf("%2c ", symbol);
        }
        SeatTier tier = getSeatTier(r);
        printf("  <- %s (Rs. %.0f)\n", getTierLabel(tier), getBasePrice(tier));
    }
    printf("\nLegend: '.' = free   'X' = booked\n");
}
void printRevenueReport(Movie movies[], Showtime shows[]) {
    printf("\n===================================================================\n");
    printf("                          REVENUE REPORT\n");
    printf("===================================================================\n");
    printf("%-20s %-10s %-15s %s\n", "Movie", "Showtime", "Tickets Sold", "Revenue");
    printf("-------------------------------------------------------------------\n");

    double grandTotal = 0.0;
    int grandTickets = 0;

    for (int m = 0; m < NUM_MOVIES; m++) {
        for (int t = 0; t < SHOWTIMES_PER_MOVIE; t++) {
            int s = m * SHOWTIMES_PER_MOVIE + t;
            printf("%-20s %-10s %-15d Rs. %.2f\n",
                   movies[m].title, movies[m].times[t],
                   shows[s].ticketsSold, shows[s].totalRevenue);
            grandTotal += shows[s].totalRevenue;
            grandTickets += shows[s].ticketsSold;
        }
    }

    printf("-------------------------------------------------------------------\n");
    printf("%-20s %-10s %-15d Rs. %.2f\n", "TOTAL", "", grandTickets, grandTotal);
    printf("===================================================================\n");
}
