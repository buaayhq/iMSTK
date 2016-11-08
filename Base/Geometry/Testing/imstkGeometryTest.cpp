#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkPlane.h"

using namespace imstk;

class imstkGeometryTest : public ::testing::Test
{
protected:
    Plane m_geometry; // Can't use imstk::Geometry since pure virtual. Should use google mock class.
};

TEST_F(imstkGeometryTest, GetSetScaling)
{
    m_geometry.setScaling(2);
    EXPECT_EQ(m_geometry.getScaling(), 2);

    m_geometry.setScaling(0.003);
    EXPECT_EQ(m_geometry.getScaling(), 0.003);

    m_geometry.setScaling(400000000);
    EXPECT_EQ(m_geometry.getScaling(), 400000000);

    m_geometry.setScaling(0);
    EXPECT_GT(m_geometry.getScaling(), 0);

    m_geometry.setScaling(-5);
    EXPECT_GT(m_geometry.getScaling(), 0);
}

TEST_F(imstkGeometryTest, GetSetPosition)
{
    auto p1 = Vec3d(12, 0.0005, -400000);
    auto p2 = Vec3d(-500, 30, 0.23);

    m_geometry.setPosition(p1);
    EXPECT_EQ(m_geometry.getPosition(), p1);

    m_geometry.setPosition(p2);
    EXPECT_EQ(m_geometry.getPosition(), p2);

    m_geometry.setPosition(p1[0], p1[1], p1[2]);
    EXPECT_EQ(m_geometry.getPosition(), p1);

    m_geometry.setPosition(p2[0], p2[1], p2[2]);
    EXPECT_EQ(m_geometry.getPosition(), p2);
}

TEST_F(imstkGeometryTest, GetSetOrientation)
{
    auto angle1 = 15;
    auto axis1 = Vec3d(12, 0, -0.5);
    auto aa1 = Rotd(angle1, axis1);
    auto q1 = Quatd(aa1);

    auto angle2 = 0.43;
    auto axis2 = Vec3d(4000, -1, 0);
    auto aa2 = Rotd(angle2, axis2);
    auto mat2 = Mat3d(aa2);
    auto q2 = Quatd(mat2);

    auto angle3 = 800;
    auto axis3 = Vec3d(-0, 100, 2000000);
    auto aa3 = Rotd(angle3, axis3);
    auto q3 = Quatd(aa3);

    // NOTE: '==' not defined for Eigen::Quaternion, using 'isApprox'.
    // See https://forum.kde.org/viewtopic.php?f=74&t=118598

    m_geometry.setOrientation(q1);
    EXPECT_TRUE(m_geometry.getOrientation().isApprox(q1));

    m_geometry.setOrientation(mat2);
    EXPECT_TRUE(m_geometry.getOrientation().isApprox(q2));

    m_geometry.setOrientation(axis3, angle3);
    EXPECT_TRUE(m_geometry.getOrientation().isApprox(q3));
}

int imstkGeometryTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
