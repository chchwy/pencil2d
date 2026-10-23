#include "vectorimage.h"
#include "catch.hpp"

TEST_CASE("VectorImage removeColor")
{
    auto vImage = VectorImage();
    auto bezier = BezierCurve({ QPoint(50,50), QPoint(100,100)});
    SECTION("Ensure that number is changed")
    {
        bezier.setColorNumber(3);
        vImage.addCurve(bezier, 1.0);

        vImage.removeColor(3);

        REQUIRE(vImage.curve(0).getColorNumber() == 2);
    }

    SECTION("Can't get below zero")
    {
        bezier.setColorNumber(0);
        vImage.addCurve(bezier, 1.0);

        vImage.removeColor(0);

        REQUIRE(vImage.curve(0).getColorNumber() == 0);
    }
}

TEST_CASE("VectorImage::getVerticesCloseTo segment query")
{
    // Two vertices: the origin at (50, 0), and a far-away one that no query below reaches.
    auto vImage = VectorImage();
    auto bezier = BezierCurve({ QPointF(50, 0), QPointF(500, 500) });
    vImage.addCurve(bezier, 1.0);

    SECTION("a point query misses a vertex the brush swept past")
    {
        // This is the bug: with the pointer moving fast, two consecutive events land at
        // (0, 8) and (100, 8). The vertex is ~50 from either one, so neither sees it,
        // even though the brush passed within 8 of it.
        REQUIRE(vImage.getVerticesCloseTo(QPointF(0, 8), 10.0).isEmpty());
        REQUIRE(vImage.getVerticesCloseTo(QPointF(100, 8), 10.0).isEmpty());
    }

    SECTION("the segment query finds it")
    {
        // Perpendicular distance from (50, 0) to the segment along y = 8 is 8.
        QList<VertexRef> found = vImage.getVerticesCloseTo(QPointF(0, 8), QPointF(100, 8), 10.0);
        REQUIRE(found.size() == 1);
        REQUIRE(vImage.getVertex(found.first()) == QPointF(50, 0));
    }

    SECTION("distance is capped at the endpoints, not measured to the infinite line")
    {
        // Same line, but the stroke stops at x = 20, so the vertex is 31 away from the
        // nearest point of the segment and must not be picked up.
        REQUIRE(vImage.getVerticesCloseTo(QPointF(0, 8), QPointF(20, 8), 10.0).isEmpty());
    }

    SECTION("a degenerate segment behaves like the point query")
    {
        REQUIRE(vImage.getVerticesCloseTo(QPointF(55, 0), QPointF(55, 0), 10.0).size() == 1);
        REQUIRE(vImage.getVerticesCloseTo(QPointF(70, 0), QPointF(70, 0), 10.0).isEmpty());
    }

    SECTION("the whole segment is swept, not just its ends")
    {
        // Walking the brush along y = 8 from far left to far right picks the vertex up
        // exactly once, wherever the events happen to land.
        REQUIRE(vImage.getVerticesCloseTo(QPointF(-400, 8), QPointF(400, 8), 10.0).size() == 1);
    }
}
