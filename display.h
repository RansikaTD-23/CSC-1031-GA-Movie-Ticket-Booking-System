#ifndef DISPLAY_H
#define DISPLAY_H

#include "movie.h"

void printMainMenu(void);
void printShowtimesList(Movie movies[]);
void printSeatMap(const Showtime *show);
void printRevenueReport(Movie movies[], Showtime shows[]);

#endif
