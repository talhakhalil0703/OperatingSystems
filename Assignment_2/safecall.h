// =======================================================================
//                        !!! DO NOT EDIT THIS FILE !!!
// =======================================================================
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#pragma once

// unsafe() is a misbehaved function that your safecall() function will
// call. This function is implemented for you, and you cannot change
// how it works.
//
// unsafe() will sometimes run correctly, an will quickly return
// an integer (in under 0.5 seconds).
//
// however, sometimes it will misbehave:
//   - occasionally, it will run for too long, e.g. 5 seconds
//   - other times it will never return
//   - and sometimes, it will crash
//
int unsafe(int);

// this is the function you need to reimplement
//
// safecall(n) calls unsafe(n) and returns the same value as
// unsafe(n), but only when unsafe(n) does not misbehave. If unsafe(n)
// misbehaves, you need to:
//
//   return -1 when unsafe(n) runs for longer than 1 second
//   return -2 when unsafe(n) crashes
//
// also, if unsafe(n) runs for more than 1s, you need to return after 1s.
int safecall( int n);