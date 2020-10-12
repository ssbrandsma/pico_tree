#include <gtest/gtest.h>

#include <pico_adaptor.hpp>
#include <pico_tree/kd_tree.hpp>
#include <scoped_timer.hpp>

#include "common.hpp"

template <typename PicoAdaptor>
using KdTree = pico_tree::KdTree<
    typename PicoAdaptor::Index,
    typename PicoAdaptor::Scalar,
    PicoAdaptor::Dims,
    PicoAdaptor>;

TEST(KdTreeTest, MetricL1) {
  using PointX = Point2f;
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  constexpr auto Dims = PointX::Dims;
  std::vector<PointX> points{{2.0f, 4.0f}};
  AdaptorX adaptor(points);
  PointX p{10.0f, 1.0f};

  pico_tree::MetricL1<Index, Scalar, Dims, AdaptorX> metric(adaptor);

  EXPECT_FLOAT_EQ(metric(p, 0), 11.0f);
  EXPECT_FLOAT_EQ(metric(-3.1f, 8.9f), 12.0f);
  EXPECT_FLOAT_EQ(metric(-3.1f), 3.1f);
}

TEST(KdTreeTest, MetricL2) {
  using PointX = Point2f;
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  constexpr auto Dims = PointX::Dims;
  std::vector<PointX> points{{2.0f, 4.0f}};
  AdaptorX adaptor(points);
  PointX p{10.0f, 1.0f};

  pico_tree::MetricL2<Index, Scalar, Dims, AdaptorX> metric(adaptor);

  EXPECT_FLOAT_EQ(metric(p, 0), 73.0f);
  EXPECT_FLOAT_EQ(metric(-3.1f, 8.9f), 144.0f);
  EXPECT_FLOAT_EQ(metric(-3.1f), 9.61f);
}

TEST(KdTreeTest, SplitterMedian) {
  using PointX = Point2f;
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  constexpr auto Dims = PointX::Dims;
  std::vector<PointX> pts4{
      {0.0f, 4.0f}, {0.0f, 2.0f}, {0.0f, 3.0f}, {0.0f, 1.0f}};
  std::vector<Index> idx4{0, 1, 2, 3};
  AdaptorX ptsx4(pts4);

  pico_tree::internal::Sequence<Scalar, 2> min;
  pico_tree::internal::Sequence<Scalar, 2> max;
  min[0] = 0.0f;
  min[1] = 0.0f;
  max[0] = 1.0f;
  max[1] = 0.0f;
  Index split_dim;
  Index split_idx;
  Scalar split_val;

  pico_tree::SplitterLongestMedian<Index, Scalar, Dims, AdaptorX> splitter4(
      ptsx4, &idx4);
  splitter4(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 0);
  EXPECT_EQ(split_idx, 2);
  EXPECT_EQ(split_val, pts4[2](0));

  std::vector<PointX> pts7{
      {3.0f, 6.0f},
      {0.0f, 4.0f},
      {0.0f, 2.0f},
      {0.0f, 5.0f},
      {0.0f, 3.0f},
      {0.0f, 1.0f},
      {1.0f, 7.0f}};
  std::vector<Index> idx7{0, 1, 2, 3, 4, 5, 6};
  AdaptorX ptsx7(pts7);

  pico_tree::SplitterLongestMedian<Index, Scalar, Dims, AdaptorX> splitter7(
      ptsx7, &idx7);
  splitter7(0, 0, 7, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 0);
  EXPECT_EQ(split_idx, 3);
  EXPECT_EQ(split_val, pts7[idx7[3]](0));

  max[1] = 10.0f;
  splitter7(1, 3, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 5);
  EXPECT_EQ(split_val, pts7[idx7[5]](1));
}

TEST(KdTreeTest, SplitterSlidingMidpoint) {
  using PointX = Point2f;
  using Index = int;
  using Scalar = typename PointX::Scalar;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  constexpr auto Dims = PointX::Dims;
  std::vector<PointX> pts4{{0.0, 2.0}, {0.0, 1.0}, {0.0, 4.0}, {0.0, 3.0}};
  std::vector<Index> idx4{0, 1, 2, 3};
  AdaptorX ptsx4(pts4);

  pico_tree::SplitterSlidingMidpoint<Index, Scalar, Dims, AdaptorX> splitter(
      ptsx4, &idx4);

  pico_tree::internal::Sequence<Scalar, 2> min;
  pico_tree::internal::Sequence<Scalar, 2> max;
  Index split_dim;
  Index split_idx;
  Scalar split_val;

  // Everything is forced to the right leaf. This means we want a single point
  // to the left (the lowest value) and internally the splitter needs to reorder
  // the indices such that on index 1 we get value 2.
  min[0] = Scalar{0.0};
  min[1] = Scalar{0.0};
  max[0] = Scalar{0.0};
  max[1] = Scalar{1.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 1);
  EXPECT_EQ(split_val, pts4[0](1));
  EXPECT_EQ(idx4[0], 1);
  EXPECT_EQ(idx4[1], 0);

  // Everything is forced to the left leaf. This means we want a single point
  // to the right (the highest value) and internally the splitter needs to
  // reorder the indices such that on index 3 we get value 4.
  max[1] = Scalar{9.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 3);
  EXPECT_EQ(split_val, pts4[2](1));
  EXPECT_EQ(idx4[3], 2);

  // Clean middle split. A general case where the split value falls somewhere
  // inbetween the range of numbers.
  max[1] = Scalar{5.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 1);
  EXPECT_EQ(split_idx, 2);
  EXPECT_EQ(split_val, (max[1] + min[1]) / Scalar{2.0});

  // On dimension 0 we test what happens when all values are equal. Again
  // everything moves to the left. So we want to split on index 3.
  max[0] = Scalar{15.0};
  splitter(0, 0, 4, min, max, &split_dim, &split_idx, &split_val);

  EXPECT_EQ(split_dim, 0);
  EXPECT_EQ(split_idx, 3);
  EXPECT_EQ(split_val, pts4[3](0));
}

// The anonymous namespace gives the function a unique "name" when there is
// another one with the exact same signature.
namespace {

template <typename PointX>
void QueryRange(
    int const point_count,
    typename PointX::Scalar const area_size,
    typename PointX::Scalar const min_v,
    typename PointX::Scalar const max_v) {
  using Index = int;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  std::vector<PointX> random = GenerateRandomN<PointX>(point_count, area_size);
  AdaptorX adaptor(random);
  KdTree<AdaptorX> tree(adaptor, 8);

  TestBox(tree, min_v, max_v);
}

template <typename PointX>
void QueryRadius(
    int const point_count,
    typename PointX::Scalar const area_size,
    typename PointX::Scalar const radius) {
  using Index = int;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  std::vector<PointX> random = GenerateRandomN<PointX>(point_count, area_size);
  AdaptorX adaptor(random);
  KdTree<AdaptorX> tree(adaptor, 8);

  TestRadius(tree, radius);
}

template <typename PointX>
void QueryKnn(
    int const point_count,
    typename PointX::Scalar const area_size,
    int const k) {
  using Index = int;
  using AdaptorX = PicoAdaptor<Index, PointX>;
  std::vector<PointX> random = GenerateRandomN<PointX>(point_count, area_size);
  AdaptorX adaptor(random);
  KdTree<AdaptorX> tree(adaptor, 8);

  TestKnn(tree, static_cast<Index>(k));
}

}  // namespace

TEST(KdTreeTest, QueryRangeSubset2d) {
  QueryRange<Point2f>(1024 * 1024, 100.0f, 15.1f, 34.9f);
}

TEST(KdTreeTest, QueryRangeAll2d) {
  QueryRange<Point2f>(1024, 10.0f, 0.0f, 10.0f);
}

TEST(KdTreeTest, QueryRadiusSubset2d) {
  QueryRadius<Point2f>(1024 * 1024, 100.0f, 2.5f);
}

TEST(KdTreeTest, QueryKnn1) { QueryKnn<Point2f>(1024 * 1024, 100.0f, 1); }

TEST(KdTreeTest, QueryKnn10) { QueryKnn<Point2f>(1024 * 1024, 100.0f, 10); }
