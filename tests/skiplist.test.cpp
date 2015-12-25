#include "skiplist.hpp"
#include <iostream>
#include <gtest/gtest.h>

using namespace guoxiao::skiplist;
TEST(SkipList, insert) {
  SkipList<std::string, std::string> s;
  auto it = s.emplace("Hello", "World");
  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(it->key, "Hello");
  EXPECT_EQ(it->value, "World");
  EXPECT_EQ(it, s.find("Hello"));
  EXPECT_EQ(s["Hello"], "World");
  s.emplace("Hello2", "World2");
  EXPECT_EQ(s.find("Hello")->value, "World");
  EXPECT_EQ(s["Hello"], "World");
  EXPECT_EQ(s.size(), 2ul);
  EXPECT_EQ(s.find("Hello2")->value, "World2");
  EXPECT_EQ(s["Hello2"], "World2");

  SkipList<std::string, std::string> s2(s);
  EXPECT_EQ(s2.size(), 2ul);
  EXPECT_TRUE((bool)s2.find("Hello"));
  EXPECT_EQ(s2.find("Hello")->value, "World");
  EXPECT_EQ(s2["Hello"], "World");
  EXPECT_EQ(s2.find("Hello2")->value, "World2");
  EXPECT_EQ(s2["Hello2"], "World2");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


