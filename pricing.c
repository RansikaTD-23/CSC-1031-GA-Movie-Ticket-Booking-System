
#include "pricing.h"

SeatTier getSeatTier(int row) {
    /* Rows A-B (index 0,1) -> Regular, C-D (2,3) -> Premium, E (4) -> VIP */
    if (row == 0 || row == 1) return TIER_REGULAR;
    if (row == 2 || row == 3) return TIER_PREMIUM;
    return TIER_VIP;
}

double getBasePrice(SeatTier tier) {
    switch (tier) {
        case TIER_REGULAR: return PRICE_REGULAR;
        case TIER_PREMIUM: return PRICE_PREMIUM;
        case TIER_VIP:      return PRICE_VIP;
    }
    return 0.0;
}

const char *getTierLabel(SeatTier tier) {
    switch (tier) {
        case TIER_REGULAR: return "Regular";
        case TIER_PREMIUM: return "Premium";
        case TIER_VIP:      return "VIP";
    }
    return "Unknown";
}

double calculatePrice(SeatTier tier, int isStudent, int isSenior, int isGroup) {
    double price = getBasePrice(tier);
    double categoryDiscountPct = 0.0;

    /* Only one category discount can apply */
    if (isSenior) {
        categoryDiscountPct = SENIOR_DISCOUNT_PCT;
    } else if (isStudent) {
        categoryDiscountPct = STUDENT_DISCOUNT_PCT;
    }

    double totalDiscountPct = categoryDiscountPct;

    /* Group discount can be combined with a category discount */
    if (isGroup) {
        totalDiscountPct += GROUP_DISCOUNT_PCT;
    }

    price = price - (price * totalDiscountPct / 100.0);
    return price;
}
