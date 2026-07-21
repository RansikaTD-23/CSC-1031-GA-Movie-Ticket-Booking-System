###### **Movie Ticket Booking System (CSC 1031 Group Project)**

A command-line C program that lets a movie theater manage seat bookings and

ticket pricing across multiple movies and showtimes, entirely in memory.



###### **Files**

|**File**|**Purpose**|
|-|-|
|movie.h|shared structs(movie, showtime, seat) and constants|
|pricing.h/pricing.c|seat tier lookup and price/discount calculation logic|
|booking.h/booking.c|booking, cancellation, validation, and search logic|
|display.h/display.c|menu, showtime list, seat map, and revenue report|
|main.c|program entry point, menu loop and input handling|



###### **How to complie**

Using gcc (Linux/macOS, or Windows with MinGW):

**gcc \*.c -o booking**



Using Code::Blocks (windows):

1. Create a New empty C project.
2. Add all .c and .h files to the project
3. Build and Run (F9)



###### **How to Run**

**./booking**

On windows **booking.exe**



###### **Features**

* View Showtimes — lists all 3 movies, each with 2 showtimes.
* View Seat Map — 5x10 grid per showtime ( . = free, X = booked),labeled by pricing tier (Regular / Premium / VIP).
* Book a Seat — supports booking multiple seats in a single transaction (seat labels like C7), with optional Student or Senior Citizen discount,and an automatic Group Discount when 4+ seats are booked together.
* Cancel a Booking — frees the seat and removes its price from revenue.
* Search Booking — by customer name (partial match) or by seat number.
* Revenue Report — tickets sold and revenue per showtime, plus grand totals.



###### **Pricing Rules**

|**Seat Tier**|**Rows**|**Base Price**|
|-|-|-|
|Regular|A - B|Rs.500|
|Premium|C - D|Rs.750|
|VIP|E|Rs.1000|



* Student discount: 10% off base price.
* Senior Citizen discount: 20% off base price.
* Only one of Student/Senior applies per booking (mutually exclusive).
* Group discount: extra 10% off when 4+ seats are booked in one transaction stacks with Student or Senior.



###### **Validation \& Error Handling**

* Rejects booking an already-taken seat.
* Rejects out-of-range seat numbers/labels.
* Rejects cancelling a seat that isn't booked.
* Non-numeric menu input is caught and re-prompted the program never crashes on bad input.



###### **Known Limitations**

* Data is in memory only nothing is saved once the program exits.
* Seat, movie data is hardcoded.

