//
// Created by Jan Drewniok on 07.02.23.
//

#include <catch2/catch_template_test_macros.hpp>

#include <fiction/algorithms/simulation/sidb/critical_temperature_exact.hpp>
#include <fiction/algorithms/simulation/sidb/energy_distribution.hpp>
#include <fiction/layouts/cartesian_layout.hpp>
#include <fiction/layouts/cell_level_layout.hpp>
#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/hexagonal_layout.hpp>
#include <fiction/technology/cell_technologies.hpp>

using namespace fiction;

TEMPLATE_TEST_CASE(
    "Test energy_distribution function", "[energy_distribution]",
    (cell_level_layout<sidb_technology, clocked_layout<cartesian_layout<siqad::coord_t>>>),
    (cell_level_layout<sidb_technology, clocked_layout<hexagonal_layout<siqad::coord_t, odd_row_hex>>>),
    (cell_level_layout<sidb_technology, clocked_layout<hexagonal_layout<siqad::coord_t, even_row_hex>>>),
    (cell_level_layout<sidb_technology, clocked_layout<hexagonal_layout<siqad::coord_t, odd_column_hex>>>),
    (cell_level_layout<sidb_technology, clocked_layout<hexagonal_layout<siqad::coord_t, even_column_hex>>>))
{

    SECTION("empty layout")
    {
        TestType lyt{{10, 10}};

        const sidb_simulation_parameters params{2, -0.32};

        critical_temperature_stats<TestType> criticalstats{};
        critical_temperature(lyt, "or", "10", params, &criticalstats);
        CHECK(criticalstats.num_valid_lyt == 0);
        CHECK(criticalstats.critical_temperature == 0);
    }

    SECTION("one SiDB placed")
    {
        TestType lyt{{10, 10}};
        lyt.assign_cell_type({0, 0}, TestType::cell_type::NORMAL);

        const sidb_simulation_parameters params{2, -0.32};

        critical_temperature_stats<TestType> criticalstats{};
        critical_temperature(lyt, "or", "10", params, &criticalstats, 0.99, 350);
        CHECK(criticalstats.num_valid_lyt == 1);
        CHECK(criticalstats.critical_temperature == 350);
    }

    SECTION("several SiDBs placed")
    {
        TestType lyt{{10, 10}};
        lyt.assign_cell_type({0, 0}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({10, 2, 1}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({5, 1, 1}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({1, 2, 4}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({7, 4, 1}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({15, 1, 1}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({1, 10, 4}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({7, 8, 1}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({1, 9, 1}, TestType::cell_type::NORMAL);
        lyt.assign_cell_type({7, 6, 1}, TestType::cell_type::NORMAL);

        const sidb_simulation_parameters params{2, -0.32};

        critical_temperature_stats<TestType> criticalstats{};
        critical_temperature(lyt, "or", "10", params, &criticalstats);
        CHECK(criticalstats.critical_temperature == 0);

        critical_temperature_stats<TestType> criticalstats_one{};
        critical_temperature(lyt, "and", "10", params, &criticalstats_one);
        CHECK(criticalstats_one.critical_temperature == 400);

        critical_temperature_stats<TestType> criticalstats_second{};
        critical_temperature(lyt, "and", "01", params, &criticalstats_second);
        CHECK(criticalstats_second.critical_temperature == 400);
    }
}