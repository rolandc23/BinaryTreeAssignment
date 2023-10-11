// Implementation of the Flight DB ADT
// z5311913 25 October 2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlightDb.h"
#include "List.h"
#include "Tree.h"

#define MAX_CHAR "zzzzzzzz"

struct flightDb {
    Tree byFlightNum;
    Tree byDepartureAirport;
    Tree byDepartureTime;
};

static int timeDifference(Record r1, Record r2);

// Comparison Functions

// Sort by flight number
// If flight numbers are the same, sort by departure time
int compareByFlightNum(Record r1, Record r2) {

    // Integer assignment comparing strings of 2 flight numbers
    int compFlightNum = strcmp(RecordGetFlightNumber(r1), RecordGetFlightNumber(r2));

    // Check cases
    if (compFlightNum != 0) {
        // If flight numbers are different
        return compFlightNum;

    } else {
        // If flight numbers are same, return time departure difference
        return timeDifference(r1, r2);
    }

}

// Sort by departure airport
// If same departure airport, then compare departure time
int compareByDepartureAirport(Record r1, Record r2) {

    // String comparison of departure airports between 2 records
    int compDepAirport = strcmp(RecordGetDepartureAirport(r1), RecordGetDepartureAirport(r2));

    // Check cases
    if (compDepAirport != 0) {
        // Different departure airport
        return compDepAirport;

    } else if (timeDifference(r1, r2) != 0) {
        // Different departure time
        return timeDifference(r1, r2);

    } else {
        // Different flight number
        return strcmp(RecordGetFlightNumber(r1), RecordGetFlightNumber(r2));
    }

}

// Sort by departure time
// If same departure time, sort by flight number
int compareByDepartureTime(Record r1, Record r2) {

    // Integer assigned for which one has earlier departure
    int t = timeDifference(r1, r2);

    // Check cases
    if (t != 0) {
        // Different departure times
        return t;

    } else {
        // Same departure times, compare flight numbers
        return strcmp(RecordGetFlightNumber(r1), RecordGetFlightNumber(r2));
    }

}

// Returns integer determining which record has earlier departure
// If negative, r1 is earlier
// If positive, r2 is earlier
// If 0, r1 and r2 have same departure time
static int timeDifference(Record r1, Record r2) {

    // Assign integer comparisons for departure day, hour and minute
    int compDepDay = RecordGetDepartureDay(r1) - RecordGetDepartureDay(r2),
        compDepHour = RecordGetDepartureHour(r1) - RecordGetDepartureHour(r2),
        compDepMin = RecordGetDepartureMinute(r1) - RecordGetDepartureMinute(r2);
    
    // Check cases
    if (compDepDay != 0) {
        // Check days
        return compDepDay;

    } else if (compDepHour != 0) {
        // Check hours
        return compDepHour;

    } else {
        // Check minutes
        return compDepMin;
    }

}

// Create new flight database
FlightDb DbNew(void) {

    FlightDb db = malloc(sizeof(*db));

    // Error case
    if (db == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    // Create trees sorted with respective terms
    db->byFlightNum = TreeNew(compareByFlightNum);
    db->byDepartureAirport = TreeNew(compareByDepartureAirport);
    db->byDepartureTime = TreeNew(compareByDepartureTime);

    return db;
}

// Frees database
void DbFree(FlightDb db) {

    // Free all trees
    TreeFree(db->byFlightNum, true);
    TreeFree(db->byDepartureAirport, false);
    TreeFree(db->byDepartureTime, false);

    free(db);

}

// Inserts record
// True if inserted
// False if insertion failed since record with same values
// has already been entered
bool DbInsertRecord(FlightDb db, Record r) {

    // Check insertion cases
    if (TreeInsert(db->byFlightNum, r)) {
        TreeInsert(db->byDepartureAirport, r);
        TreeInsert(db->byDepartureTime, r);
        return true;
    } else {
        return false;
    }

}

// Finds records of a certain flight number
// Returns list ordered in departure times
// Returns empty list if no such records exist
List DbFindByFlightNumber(FlightDb db, char *flightNumber) {

    // Form dummy records
    Record dummyLower = RecordNew(flightNumber, "", "", 0, 0, 0, 0),
           dummyUpper = RecordNew(flightNumber, "", "", 6, 23, 59, 9999);
    
    // Search between dummy bounds and returns list
    List l = TreeSearchBetween(db->byFlightNum, dummyLower, dummyUpper);

    // Free dummy records
    RecordFree(dummyLower);
    RecordFree(dummyUpper);

    return l;

}

// Find records based on departure airport and day of departure
// Returns list in order of hour, min then flight number
// Returns empty list if no such records exist
List DbFindByDepartureAirportDay(FlightDb db, char *departureAirport,
                                 int day) {

    // Create dummy records                                 
    Record dummyLower = RecordNew("", departureAirport, "", day, 0, 0, 0),
           dummyUpper = RecordNew(MAX_CHAR, departureAirport, "", day, 23, 59, 9999);
    
    // Search between lower and upper bounds, assorted in departure airport times
    List l = TreeSearchBetween(db->byDepartureAirport, dummyLower, dummyUpper);

    // Free dummy records
    RecordFree(dummyLower);
    RecordFree(dummyUpper);

    return l;
}

// Finds records of all flights with departure time between
// 2 given times during the week
// Returned list is ordered by departure time then flight number
// Returns empty list if no such records exist
List DbFindBetweenTimes(FlightDb db, 
                        int day1, int hour1, int min1, 
                        int day2, int hour2, int min2) {
    // Integer values used to compare which time (1 or 2) 
    // is earlier in the week
    int a = day1 * 24 * 60 + hour1 * 60 + min1,
        b = day2 * 24 * 60 + hour2 * 60 + min2;
    
    // Check cases
    if (a > b) {
        // If lower bound is right of upper bound
        // Perform outward search from left-most node to
        // upper bound, and lower bound to right-most node
        Record dummyLower = RecordNew("", "", "", day1, hour1, min1, 0),
               dummyUpper = RecordNew(MAX_CHAR, "", "", day2, hour2, min2, 0),
               dummyEdgeL = RecordNew("", "", "", 0, 0, 0, 0),
               dummyEdgeR = RecordNew(MAX_CHAR, "", "", 6, 23, 59, 9999);
        
        // List containing left-side records
        List res = TreeSearchBetween(db->byDepartureTime, dummyLower, dummyEdgeR),
        // List containing right-side records
             save = TreeSearchBetween(db->byDepartureTime, dummyEdgeL, dummyUpper);

        // Append left-side record list to right-side record
        ListExtend(res, save);

        // Free dummy records
        RecordFree(dummyLower);
        RecordFree(dummyUpper);
        RecordFree(dummyEdgeL);
        RecordFree(dummyEdgeR);

        // Free right-side record list
        ListFree(save);

        return res;

    } else {
        // Otherwise perform inward search between lower and upper bounds
        Record dummyLower = RecordNew("", "", "", day1, hour1, min1, 0),
               dummyUpper = RecordNew(MAX_CHAR, "", "", day2, hour2, min2, 0);
        
        // List of records between bounds
        List l = TreeSearchBetween(db->byDepartureTime, dummyLower, dummyUpper);

        // Free dummy records
        RecordFree(dummyLower);
        RecordFree(dummyUpper);

        return l;
    }

}

// Returns record which is the earlist next flight with
// given flight number, on or after the given (day, hour min)
// Returns NULL if no flights of such description exist
Record DbFindNextFlight(FlightDb db, char *flightNumber, 
                        int day, int hour, int min) {
    
    // Assign dummy record containing record with flight number
    // and given (day, hour, min)
    Record dummy = RecordNew(flightNumber, "", "", day, hour, min, 0),
    // Search record next to given record
           search = TreeNext(db->byFlightNum, dummy);

    // Check cases
    if (search != NULL && strcmp(flightNumber, RecordGetFlightNumber(search)) == 0) {
        // If search returned a record and record matches flight number
        RecordFree(dummy);
        return search;

    } else {
        // If search returned a record and record doesn't match flight number
        // OR if search was null

        // Create dummy with left-most bound for given flight number
        Record dummyLeft = RecordNew(flightNumber, "", "", 0, 0, 0, 0),
        // Search for record next to dummyLeft
               res = TreeNext(db->byFlightNum, dummyLeft);

        // Free dummy records
        RecordFree(dummy);
        RecordFree(dummyLeft);

        // Checks if search returned a record but flight numbers don't match
        if (res != NULL && strcmp(flightNumber, RecordGetFlightNumber(res)) != 0) {
            return NULL;
        }

        return res;
    }

}