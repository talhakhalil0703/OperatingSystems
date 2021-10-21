// =======================================================================
//                        !!! DO NOT EDIT THIS FILE !!!
// =======================================================================
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "safecall.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <functional>

namespace {
// input file format for creating unsafe() function
// 
// the file contains a list of integers seperated by one or more white spaces
//
// number    action
// -----------------
// n >  0  function immediatlly returns n
// n =  0  after 0.5s, function returns the input
// n = -1  after 5s, function returns the input
// n = -2  function never returns (infinite loop)
// n < -2  function crashes

std::function<int(int)> mkfun( const std::vector<int> & correct)
{
  return [=] (int p) -> int {
    if( p < 0 || p >= int(correct.size()))
      return -1;
    int n = correct[p];
    if( n > 0) {
      return n;
    } else if( n == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      return p;
    } else if( n == -1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
      return p;
    } else if( n == -2) {
      while(1)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      exit(p);
    }
  };
}

struct {
  std::function<int(int)> unsafe;
} globals;

void usage(const std::string & pname)
{
  std::cout
      << "Usage: " << pname << " [verbocity]\n"
      << "Verbocity controls which outputs to display\n"
      << "\n"
      << "    none = do not display any outputs\n"
      << "    auto = display only wrong outputs\n"
      << "    all  = display all outputs\n"
      << "\n"
      << "The default verbocity is set to 'auto'\n"
      << "\n"
      << "The input is a list of integers, used to construct the unsafe()\n"
      << "function. For i-th positive integer n, unsafe(i) = n. For\n"
      << "integers <= 0 the unsafe() function will have different\n"
      << "behavior, as described below:\n"
      << "\n"
      << "    n >  0  : function immediatlly returns 'n'\n"
      << "    n =  0  : function delays for 0.5s, then returns 'i'\n"
      << "    n = -1  : function delays for 5s, then returns 'i'\n"
      << "    n = -2  : function never returns (enters infinite loop)\n"
      << "    n < -2  : function crashes or exits the process\n"
      << "\n";
  exit(-1);
}

std::vector<int> safecalls( int n, bool debug)
{
  std::vector<int> result;
  for( int i = 0 ; i < n ; i ++ ) {
    if( debug)
      std::cout << "Calling safecall(" << i << ")\n" << std::flush;
    auto t1 = std::chrono::steady_clock::now();
    int c = safecall(i);
    auto t2 = std::chrono::steady_clock::now();
    double tdiff = std::chrono::duration_cast<std::chrono::milliseconds>
      (t2 - t1).count() / 1000.0;
    if(debug)
      std::cout << "   safecall(" << i << ") returned " << c
                << " after " << tdiff << "s\n" << std::flush;
    result.push_back(c);
  }
  
  return result;
}
} // end of anonymous namespace

int unsafe(int p) {
  return ::globals.unsafe(p);
}

int main(int argc, char ** argv)
{
  int verbose = 1;
  if( argc == 1) {;}
  else if( argc == 2) {
    std::string w = argv[1];
    if( w == "none") verbose = 0;
    else if( w == "auto") verbose = 1;
    else if( w == "all") verbose = 2;
    else usage(argv[0]);
  }
  else { usage(argv[0]); }
  
  // read entire stdin into memory
  std::vector<int> input;
  while( true) {
    int n;
    if( not( std::cin >> n)) break;
    input.push_back(n);
  }

  auto correct = input;
  for( size_t i = 0 ; i < correct.size() ; i ++ ) {
    auto & c = correct[i];
    if( c > 0) continue;
    else if( c == 0) c = i;
    else if( c == -1) c = -1;
    else if( c == -2) c = -1;
    else c = -2;
  }

  // prepare the unsafe function
  ::globals.unsafe = mkfun(input);

  // call safecalls()
  auto start_time = std::chrono::steady_clock::now();
  std::vector<int> result = ::safecalls(input.size(), verbose == 2);
  auto finish_time = std::chrono::steady_clock::now();
  double tdiff = std::chrono::duration_cast<std::chrono::milliseconds>
      (finish_time - start_time).count() / 1000.0;

  std::cout << "\n";

  // print outputs
  if( verbose == 2 or (verbose == 1 and result != correct)) {
    //  if( verbose > 0) {
    if( verbose == 1) std::cout << "Only displaying outputs with errors\n";
    else std::cout << "Displaying all outputs\n";

    std::cout << std::string(50, '=') << "\n"
              << "      | Expected   | Observed\n"
              << "Index | Output     | Output\n"
              << std::string(50, '-') << "\n";
    for( size_t i = 0 ; i < std::max(correct.size(), result.size()) ; i ++) {
      std::string comment;
      if( i >= correct.size()) comment = "extra output";
      else if( i >= result.size()) comment = "missing output";
      else if( correct[i] != result[i]) comment = "wrong";

      if( comment.empty() and verbose == 1) continue;
    
      std::cout << std::setw(5) << std::right << i << " | ";
      if( i < correct.size()) 
        std::cout << std::setw(10) << std::left << correct[i] << " | ";
      else
        std::cout << "n/a       " << " | ";
      if( i < result.size()) 
        std::cout << std::setw(10) << std::left << result[i];
      else
        std::cout << "n/a       ";
      if( not comment.empty()) std::cout << " ! " << comment;
      std::cout << "\n";
    }
    std::cout << std::string(50, '=') << "\n";
  }

  // report summary 
  std::cout << "Finished in " << tdiff << "s\n";
  if( correct != result) {
    std::cout << "Wrong results :(\n";
  }
  else {
    std::cout << "Correct results, good job.\n";
  }
  return 0;
}
