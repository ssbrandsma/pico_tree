#include <gtest/gtest.h>

#include <pico_point_set.hpp>
#include <pico_tree/kd_tree.hpp>
#include <scoped_timer.hpp>

#include "common.hpp"

TEST(KdTreeTest, SplitterMedian) {
  using PointX = Point2f;
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using PointsX = PicoPointSet<Index, PointX>;
  constexpr auto Dims = PointX::Dims;
  std::vector<PointX> pts4{{0.0, 4.0}, {0.0, 2.0}, {0.0, 3.0}, {0.0, 1.0}};
  std::vector<Index> idx4{0, 1, 2, 3};
  PointsX ptsx4(pts4);

  pico_tree::internal::Sequence<Scalar, 2> min;
  pico_tree::internal::Sequence<Scalar, 2> max;
  Index split_dim;
  Index split_idx;
  Scalar split_val;

  pico_tree::SplitterMedian<Index, Scalar, Dims, PointsX> splitter4(
      ptsx4, &idx4);
  splitter4(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 0);
  EXPECT_EQ(split_idx, 2);
  EXPECT_EQ(split_val, pts4[2](0));

  std::vector<PointX> pts7{
      {3.0, 6.0},
      {0.0, 4.0},
      {0.0, 2.0},
      {0.0, 5.0},
      {0.0, 3.0},
      {0.0, 1.0},
      {1.0, 7.0}};
  std::vector<Index> idx7{0, 1, 2, 3, 4, 5, 6};
  PointsX ptsx7(pts7);

  pico_tree::SplitterMedian<Index, Scalar, Dims, PointsX> splitter7(
      ptsx7, &idx7);
  splitter7(0, 0, 7, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 0);
  EXPECT_EQ(split_idx, 3);
  EXPECT_EQ(split_val, pts7[idx7[3]](0));

  splitter7(1, 3, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 5);
  EXPECT_EQ(split_val, pts7[idx7[5]](1));
}

TEST(KdTreeTest, SplitterSlidingMidpoint) {
  using PointX = Point2f;
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using PointsX = PicoPointSet<Index, PointX>;
  constexpr auto Dims = PointX::Dims;
  std::vector<PointX> pts4{{0.0, 1.0}, {0.0, 2.0}, {0.0, 3.0}, {0.0, 4.0}};
  std::vector<Index> idx4{0, 1, 2, 3};
  PointsX ptsx4(pts4);

  pico_tree::SplitterSlidingMidpoint<Index, Scalar, Dims, PointsX> splitter(
      ptsx4, &idx4);

  pico_tree::internal::Sequence<Scalar, 2> min;
  pico_tree::internal::Sequence<Scalar, 2> max;
  Index split_dim;
  Index split_idx;
  Scalar split_val;

  min[0] = Scalar{0.0};
  min[1] = Scalar{0.0};
  max[0] = Scalar{0.0};
  max[1] = Scalar{1.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 1);
  EXPECT_EQ(split_val, pts4[1](1));

  max[1] = Scalar{9.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 3);
  EXPECT_EQ(split_val, pts4[3](1));

  max[1] = Scalar{5.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 2);
  EXPECT_EQ(split_val, (max[1] + min[1]) / Scalar{2.0});

  max[0] = Scalar{15.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 0);
  EXPECT_EQ(split_idx, 3);
  EXPECT_EQ(split_val, pts4[3](0));
}

template <typename PointX>
void QueryRange(
    int const point_count,
    typename PointX::Scalar const area_size,
    typename PointX::Scalar const min_v,
    typename PointX::Scalar const max_v) {
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using PointsX = PicoPointSet<Index, PointX>;
  std::vector<PointX> random = GenerateRandomN<PointX>(point_count, area_size);
  PointsX points(random);
  KdTree<PointsX> tree(points, 8);
  using Scalar = typename PointX::Scalar;
  TestRange(tree, min_v, max_v);
}

TEST(KdTreeTest, QuerySubset3d) {
  QueryRange<Point3f>(1024 * 1024, 100, 15.1, 34.9);
}

TEST(KdTreeTest, QueryAll3d) { QueryRange<Point3f>(1024, 10.0, 0.0, 10.0); }