#ifndef PRICING_H
#define PRICING_H

#include "movie.h"

/* Determine seat tier from row index (0 = A, 4 = E) */
SeatTier getSeatTier(int row);

/* Return the base price for a tier */
double getBasePrice(SeatTier tier);

/* Return a display label for a tier */
const char *getTierLabel(SeatTier tier);

/*
 * Calculate final ticket price.
 * isStudent and isSenior are mutually exclusive category flags (only one may be 1).
 * isGroup applies when the transaction books >= GROUP_MIN_SEATS seats.
 */
double calculatePrice(SeatTier tier, int isStudent, int isSenior, int isGroup);

#endif
