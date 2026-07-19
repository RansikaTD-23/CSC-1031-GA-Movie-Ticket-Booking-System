#ifndef BOOKING_H
#define BOOKING_H

#include "movie.h"

/* Initialize all seats in all showtimes to free/empty state */
void initShowtimes(Showtime shows[]);

/*
 * Attempt to book a single seat within a multi-seat transaction.
 * row, col are 0-indexed. isGroup reflects whether the whole transaction
 * has >= GROUP_MIN_SEATS seats.
 * Returns 1 on success, 0 if the seat is already booked.
 */
int bookSeat(Showtime *show, int row, int col, const char *name,
             int isStudent, int isSenior, int isGroup, double *priceOut);

/*
 * Cancel a booking at row, col (0-indexed).
 * Returns 1 on success, 0 if the seat was not booked.
 */
int cancelSeat(Showtime *show, int row, int col);

/* Returns 1 if row/col are within valid seat bounds */
int isValidSeat(int row, int col);

/*
 * Search for bookings by customer name (case-insensitive substring match)
 * across a single showtime. Prints results directly. Returns number found.
 */
int searchByName(const Showtime *show, const char *name);

/*
 * Search for a booking by seat number within a showtime.
 * Returns 1 if found and booked (prints details), 0 otherwise.
 */
int searchBySeat(const Showtime *show, int row, int col);

#endif
