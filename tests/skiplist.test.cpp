#include "skiplist.hpp"
#include <iostream>
#include <gtest/gtest.h>

using namespace guoxiao::skiplist;
TEST(SkipList, basic) {
  SkipList<std::string, std::string> s;
  EXPECT_EQ(s.size(), 0ul);
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.begin(), s.end());
  EXPECT_EQ(s.cbegin(), s.cend());
  EXPECT_EQ(s.level(), 0ul);
}

TEST(SkipList, emplace) {
  SkipList<std::string, std::string> s;
  auto it = s.emplace("Hello", "World");
  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(it->key, "Hello");
  EXPECT_EQ(it->value, "World");
}

TEST(SkipList, insert) {
  SkipList<std::string, std::string> s;
  std::pair<std::string, std::string> item("Hello", "World");
  auto it = s.insert(item);
  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(it->key, "Hello");
  EXPECT_EQ(it->value, "World");
  EXPECT_THROW(s.insert(item), std::runtime_error);
}

TEST(SkipList, insert_move) {
  SkipList<std::string, std::string> s;
  std::pair<std::string, std::string> item("Hello", "World");
  auto it = s.insert(std::move(item));
  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(it->key, "Hello");
  EXPECT_EQ(it->value, "World");
}

TEST(SkipList, find) {
  SkipList<std::string, std::string> s;
  auto it = s.emplace("Hello", "World");
  EXPECT_EQ(it, s.find("Hello"));
  EXPECT_EQ(s.find("Hello")->value, "World");

  s.emplace("Hello2", "World2");
  EXPECT_EQ(s.size(), 2ul);
  EXPECT_EQ(s.find("Hello2")->value, "World2");
  EXPECT_EQ(s.find("Hello3"), s.end());
}

TEST(SkipList, index) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");
  EXPECT_EQ(s["Hello"], "World");
  s.emplace("Hello2", "World2");
  EXPECT_EQ(s["Hello"], "World");
  EXPECT_EQ(s.size(), 2ul);
  EXPECT_EQ(s["Hello2"], "World2");
  s["Hello2"] = "World3";
  EXPECT_EQ(s["Hello2"], "World3");
  EXPECT_EQ(s["Hello3"].empty(), true);
}

TEST(SkipList, at) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");
  EXPECT_EQ(s.at("Hello"), "World");
  s.emplace("Hello2", "World2");
  EXPECT_EQ(s.at("Hello"), "World");
  EXPECT_EQ(s.size(), 2ul);
  EXPECT_EQ(s.at("Hello2"), "World2");
  s.at("Hello2") = "World3";
  EXPECT_EQ(s.at("Hello2"), "World3");
  EXPECT_THROW(s.at("Hello3"), std::out_of_range);
}

TEST(SkipList, emplace_move) {
  SkipList<std::string, std::string> s;
  std::string k("Movable");
  std::string v("MovableValue");
  auto it2 = s.emplace(std::move(k), std::move(v));
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
  auto it2 = s.emplace("Hello", "World");
  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(it2->value, "World");
  EXPECT_EQ(s.find("Hello"), it2);
  EXPECT_EQ(s["Hello"], "World");
  s.erase("Hello");
  EXPECT_EQ(s.size(), 0ul);
  EXPECT_THROW(s.erase("Hello"), std::out_of_range);
}

TEST(SkipList, copy_ctor) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2(s);

  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(s2.size(), 1ul);
}

TEST(SkipList, move_ctor) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2(std::move(s));

  EXPECT_EQ(s.size(), 0ul);
  EXPECT_EQ(s2.size(), 1ul);
}

TEST(SkipList, copy_assign) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2;
  s2 = s;

  EXPECT_EQ(s.size(), 1ul);
  EXPECT_EQ(s2.size(), 1ul);
}

TEST(SkipList, move_assign) {
  SkipList<std::string, std::string> s;
  s.emplace("Hello", "World");

  SkipList<std::string, std::string> s2;
  s2 = std::move(s);

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


