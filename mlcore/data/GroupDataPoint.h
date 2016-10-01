/*
 * DataPointGroup.h
 *
 *  Created on: 2016-06-07
 *      Author: Anton Kochnev
 */

#ifndef GROUP_DATA_POINT_H_
#define GROUP_DATA_POINT_H_

namespace mlcore
{
  class DataPointGroup
  {
  public:
    struct DataPoint
    {
      std::size_t values[CommonLength];
    };

  public:
    DataPointGroup(
      std::size_t points_count,
      DataPoint* points,
      std::size_t* common_part)
    :
      points_count_(points_count),
      points_(points),
      common_part_(common_part)
    {}

    std::size_t count() const
    {
      return points_count_;
    }

    std::size_t value(std::size_t point_num, std::size_t inx) const
    {
      return (inx < CommonLength ? common_part_[inx] : points_[point_num].values[inx - CommonLength]);
    }

    std::size_t& value(std::size_t point_num, std::size_t inx)
    {
      return (inx < CommonLength ? common_part_[inx] : points_[point_num].values[inx - CommonLength]);
    }

  private:
    std::size_t points_count_;
    DataPoint* points_;
    std::size_t* common_part_;
  };
}

#endif // GROUP_DATA_POINT_H_
