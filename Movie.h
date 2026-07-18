#ifndef MOVIE_H
#define MOVIE_H

/* ===================== Configuration Constants ===================== */
#define NUM_MOVIES        3
#define SHOWTIMES_PER_MOVIE 2
#define NUM_SHOWS         (NUM_MOVIES * SHOWTIMES_PER_MOVIE)

#define ROWS              5   /* A..E */
#define SEATS_PER_ROW     10  /* 1..10 */
#define TOTAL_SEATS       (ROWS * SEATS_PER_ROW)

#define MAX_NAME_LEN      40

/* Base prices (Rs.) */
#define PRICE_REGULAR     500
#define PRICE_PREMIUM     750
#define PRICE_VIP         1000

/* Discount percentages */
#define STUDENT_DISCOUNT_PCT   10
#define SENIOR_DISCOUNT_PCT    20
#define GROUP_DISCOUNT_PCT     10
#define GROUP_MIN_SEATS        4

typedef enum { TIER_REGULAR, TIER_PREMIUM, TIER_VIP } SeatTier;

typedef struct {
    int booked;                 /* 0 = free, 1 = booked */
    char customerName[MAX_NAME_LEN];
    double pricePaid;
    int isGroupBooking;         /* 1 if this seat was part of a >=4-seat transaction */
} Seat;

typedef struct {
    Seat seats[ROWS][SEATS_PER_ROW];
    int ticketsSold;
    double totalRevenue;
} Showtime;

typedef struct {
    char title[MAX_NAME_LEN];
    char times[SHOWTIMES_PER_MOVIE][10]; /* e.g. "10:00 AM" */
} Movie;

#endif
