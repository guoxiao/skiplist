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

  std::string k("Movable");
  std::string v("MovableValue");
  auto it2 = s2.emplace(std::move(k), std::move(v));
  EXPECT_EQ(it2->key, "Movable");
  EXPECT_TRUE(k.empty());
  EXPECT_TRUE(v.empty());
}

TEST(SkipList, erase) {
  SkipList<std::string, std::string> s;
  auto it = s.emplace("Hello", "World");
  EXPECT_EQ(s.size(), 1ul);
  s.erase(it);
  EXPECT_EQ(s.size(), 0ul);
  s.emplace("Hello", "World");
  EXPECT_EQ(s.size(), 1ul);
  s.erase("Hello");
  EXPECT_EQ(s.size(), 0ul);
}

TEST(SkipList, copy_assign) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2 = s;

  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(s2.size(), 1ul);
}

TEST(SkipList, move_assign) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2 = std::move(s);

  EXPECT_EQ(s.size(), 0ul);
  EXPECT_EQ(s2.size(), 1ul);
}

TEST(SkipList, move) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2(std::move(s));

  EXPECT_EQ(s.size(), 0ul);
  EXPECT_EQ(s2.size(), 1ul);
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


