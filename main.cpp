
#include "variant.hpp"

#include <vector>

/***************************************************************************/
int test_idx = 0;

#define RUN_TEST(item, type, expected) { \
   bool ok; \
   try { \
      std::cout << "#" << test_idx << ": cast from \"" \
         << (item).type_name() << "\" to \"" #type << "\" - "; \
      (item).get<type>(); \
      ok = true; \
   } catch (const std::exception&) { \
      ok = false; \
   } \
   test_idx++; \
   std::cout << std::boolalpha << "\t\t" << ((expected)==ok) << std::endl; \
}

/***************************************************************************/

struct type {
   std::string str;
   type(const char* str)
      :str(str)
   {}

   type(const type& t)
      :str(t.str)
   {}

private:
   type& operator=(const type&);
};

std::ostream& operator<< (std::ostream& os, const type& t) {
   return os << t.str;
}

/***************************************************************************/

int main() {
   try {
      std::vector<variant> arr;
      arr.emplace_back('0');
      arr.emplace_back(31);
      arr.emplace_back(32u);
      arr.emplace_back(33.48);
      arr.emplace_back(std::string("std::string"));
      arr.emplace_back(new std::string("ptr to std::string"));
      arr.emplace_back(type("string"));

      for ( auto it: arr ) {
         it.dump(std::cout) << std::endl;
      }
      std::cout << std::endl;

      RUN_TEST(arr[0], char, true);             // 0 - true
      RUN_TEST(arr[0], signed char, false);     // 1 - false
      RUN_TEST(arr[0], unsigned char, false);   // 2 - false

      RUN_TEST(arr[1], int, true);              // 3 - true
      RUN_TEST(arr[1], unsigned int, false);    // 4 - false

      RUN_TEST(arr[2], int, false);             // 5 - false
      RUN_TEST(arr[2], unsigned int, true);     // 6 - true

      RUN_TEST(arr[3], int, false);             // 7 - false
      RUN_TEST(arr[3], long, false);            // 8 - false
      RUN_TEST(arr[3], float, false);           // 9 - false
      RUN_TEST(arr[3], double, true);           // 10 - true

      RUN_TEST(arr[4], std::exception, false);  // 11 - false
      RUN_TEST(arr[4], std::ostream, false);    // 12 - false
      RUN_TEST(arr[4], std::string, true);      // 13 - true
      RUN_TEST(arr[4], std::string*, false);    // 14 - false

      RUN_TEST(arr[5], void*, true);            // 15 - true
      RUN_TEST(arr[5], char*, true);            // 16 - true
      RUN_TEST(arr[5], std::string*, true);     // 17 - true
      RUN_TEST(arr[5], std::string, false);     // 18 - false

      RUN_TEST(arr[6], int, false);             // 19 - false
      RUN_TEST(arr[6], std::string, false);     // 20 - false
      RUN_TEST(arr[6], type*, false);           // 21 - false
      RUN_TEST(arr[6], type, true);             // 22 - true

   } catch ( const std::exception& ex ) {
      std::cout << "[exception] " << ex.what() << std::endl;
   }
}

/***************************************************************************/
