#include "test_framework.h"
#include "../include/ciary.h"

void test_leap_year() {
    TEST_CASE("Leap Year Detection");
    
    // Test typical leap years
    ASSERT_TRUE(is_leap_year(2000), "2000 is a leap year");
    ASSERT_TRUE(is_leap_year(2004), "2004 is a leap year");
    ASSERT_TRUE(is_leap_year(2020), "2020 is a leap year");
    ASSERT_TRUE(is_leap_year(2024), "2024 is a leap year");
    
    // Test non-leap years
    ASSERT_FALSE(is_leap_year(1900), "1900 is not a leap year (divisible by 100, not 400)");
    ASSERT_FALSE(is_leap_year(2001), "2001 is not a leap year");
    ASSERT_FALSE(is_leap_year(2019), "2019 is not a leap year");
    ASSERT_FALSE(is_leap_year(2021), "2021 is not a leap year");
    
    // Test century years
    ASSERT_TRUE(is_leap_year(1600), "1600 is a leap year (divisible by 400)");
    ASSERT_TRUE(is_leap_year(2000), "2000 is a leap year (divisible by 400)");
    ASSERT_FALSE(is_leap_year(1700), "1700 is not a leap year");
    ASSERT_FALSE(is_leap_year(1800), "1800 is not a leap year");
    ASSERT_FALSE(is_leap_year(1900), "1900 is not a leap year");
}

void test_days_in_month() {
    TEST_CASE("Days in Month Calculation");
    
    // Test regular months in non-leap year
    ASSERT_EQ(31, days_in_month(1, 2023), "January has 31 days");
    ASSERT_EQ(28, days_in_month(2, 2023), "February has 28 days in non-leap year");
    ASSERT_EQ(31, days_in_month(3, 2023), "March has 31 days");
    ASSERT_EQ(30, days_in_month(4, 2023), "April has 30 days");
    ASSERT_EQ(31, days_in_month(5, 2023), "May has 31 days");
    ASSERT_EQ(30, days_in_month(6, 2023), "June has 30 days");
    ASSERT_EQ(31, days_in_month(7, 2023), "July has 31 days");
    ASSERT_EQ(31, days_in_month(8, 2023), "August has 31 days");
    ASSERT_EQ(30, days_in_month(9, 2023), "September has 30 days");
    ASSERT_EQ(31, days_in_month(10, 2023), "October has 31 days");
    ASSERT_EQ(30, days_in_month(11, 2023), "November has 30 days");
    ASSERT_EQ(31, days_in_month(12, 2023), "December has 31 days");
    
    // Test February in leap year
    ASSERT_EQ(29, days_in_month(2, 2024), "February has 29 days in leap year");
    ASSERT_EQ(29, days_in_month(2, 2000), "February has 29 days in leap year 2000");
    
    // Test edge cases
    ASSERT_EQ(28, days_in_month(2, 1900), "February has 28 days in 1900 (not leap year)");
}

void test_day_of_week() {
    TEST_CASE("Day of Week Calculation");
    
    // Test known dates (using 0=Sunday format)
    ASSERT_EQ(0, day_of_week(2024, 1, 7), "January 7, 2024 is Sunday");
    ASSERT_EQ(1, day_of_week(2024, 1, 1), "January 1, 2024 is Monday");
    ASSERT_EQ(2, day_of_week(2024, 1, 2), "January 2, 2024 is Tuesday");
    ASSERT_EQ(3, day_of_week(2024, 1, 3), "January 3, 2024 is Wednesday");
    ASSERT_EQ(4, day_of_week(2024, 1, 4), "January 4, 2024 is Thursday");
    ASSERT_EQ(5, day_of_week(2024, 1, 5), "January 5, 2024 is Friday");
    ASSERT_EQ(6, day_of_week(2024, 1, 6), "January 6, 2024 is Saturday");
    
    // Test historical dates
    ASSERT_EQ(0, day_of_week(2000, 1, 2), "January 2, 2000 is Sunday");
    ASSERT_EQ(4, day_of_week(1970, 1, 1), "January 1, 1970 is Thursday (Unix epoch)");
}

void test_date_compare() {
    TEST_CASE("Date Comparison");
    
    date_t date1 = {2024, 1, 1};
    date_t date2 = {2024, 1, 1};
    date_t date3 = {2024, 1, 2};
    date_t date4 = {2024, 2, 1};
    date_t date5 = {2025, 1, 1};
    
    // Test equal dates
    ASSERT_EQ(0, date_compare(date1, date2), "Same dates should be equal");
    
    // Test different days
    ASSERT_TRUE(date_compare(date1, date3) < 0, "Earlier day should be less");
    ASSERT_TRUE(date_compare(date3, date1) > 0, "Later day should be greater");
    
    // Test different months
    ASSERT_TRUE(date_compare(date1, date4) < 0, "Earlier month should be less");
    ASSERT_TRUE(date_compare(date4, date1) > 0, "Later month should be greater");
    
    // Test different years
    ASSERT_TRUE(date_compare(date1, date5) < 0, "Earlier year should be less");
    ASSERT_TRUE(date_compare(date5, date1) > 0, "Later year should be greater");
}

void test_is_today() {
    TEST_CASE("Is Today Check");
    
    date_t today = get_current_date();
    ASSERT_TRUE(is_today(today), "Current date should be today");
    
    date_t yesterday = today;
    yesterday.day--;
    if (yesterday.day == 0) {
        yesterday.month--;
        if (yesterday.month == 0) {
            yesterday.month = 12;
            yesterday.year--;
        }
        yesterday.day = days_in_month(yesterday.month, yesterday.year);
    }
    ASSERT_FALSE(is_today(yesterday), "Yesterday should not be today");
    
    date_t tomorrow = today;
    tomorrow.day++;
    int max_days = days_in_month(tomorrow.month, tomorrow.year);
    if (tomorrow.day > max_days) {
        tomorrow.day = 1;
        tomorrow.month++;
        if (tomorrow.month > 12) {
            tomorrow.month = 1;
            tomorrow.year++;
        }
    }
    ASSERT_FALSE(is_today(tomorrow), "Tomorrow should not be today");
}

void run_utils_tests() {
    TEST_SUITE("Utility Functions");
    
    test_leap_year();
    test_days_in_month();
    test_day_of_week();
    test_date_compare();
    test_is_today();
}