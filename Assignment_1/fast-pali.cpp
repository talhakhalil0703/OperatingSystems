#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>

// global variable used in stdin_readline()
//Since l2 cache is 2Mib
char buffer[2*1024*1024];
int buff_size = 0;
int buff_pos = 0;
bool end_of_file = false;

//Adapted from https://gitlab.com/cpsc457/public/longest-int-my-getchar/-/blob/master/fast-int.cpp

int read_characters()
{
  if( buff_pos >= buff_size) {
    buff_size = read( STDIN_FILENO, buffer, sizeof(buffer));
    if(buff_size <= 0) return -1;
    buff_pos = 0;
  }
  return buffer[buff_pos++];
}

// reads in a line from STDIN
// reads until any non white space character
std::string stdin_readline()
{
  std::string result;
    while( true) {
      int c = read_characters();
      if (c == -1) {
        end_of_file = true;
        break;
      }
      if(isspace(c)) break;
      result.push_back(c);
  }
  return result;
}

// returns true if a word is palindrome
bool is_palindrome( const std::string & s)
{
  for( size_t i = 0 ; i < s.size() / 2 ; i ++)
    if( tolower(s[i]) != tolower(s[s.size()-i-1]))
      return false;
  return true;
}

// returns the longest palindrome on standard input
// in case of ties for length, returns the first palindrome
// all input is broken into words, and each word is checked
// word is a sequence of characters separated by white space
// white space is whatever isspace() says it is
//    i.e. ' ', '\n', '\r', '\t', '\n', '\f'
std::string get_longest_palindrome()
{
  std::string max_pali;
  while(1) {
    std::string word = stdin_readline();
    if(end_of_file){
      if((word.size() > max_pali.size()))
      {
        if( is_palindrome(word))
            max_pali = word;
      } 
        break;
    }
    if((word.size() <= max_pali.size())) continue;
    if( is_palindrome(word))
      max_pali = word;
  }
  return max_pali;
}

int main()
{
  std::string max_pali = get_longest_palindrome();
  printf("Longest palindrome: %s\n", max_pali.c_str());
  return 0;
}

