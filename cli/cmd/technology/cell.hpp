//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_CMD_CELL_HPP
#define FICTION_CMD_CELL_HPP

#include <fiction/algorithms/physical_design/apply_gate_library.hpp>
#include <fiction/technology/qca_one_library.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>

#include <optional>
#include <string>

namespace alice
{
/**
 * Converts a gate-level layout to a cell-level layout. The conversion process allows to choose from supported FCN gate
 * libraries.
 */
class cell_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit cell_command(const environment::ptr& e) :
            command(e, "Compiles the current gate layout in store down to a cell-level layout. A gate library must be "
                       "specified in order to instruct the algorithm how to map gate tiles to cell blocks.")
    {
        add_option("--library,-l", library, "Gate library to use for mapping", true)
            ->set_type_name("{QCA-ONE=0, ToPoliNano=1, Bestagon=2}");
    }

  protected:
    /**
     * Function to perform the conversion call. Generates a cell-level layout.
     */
    void execute() override
    {
        auto& s = store<fiction::gate_layout_t>();

        // error case: empty gate layout store
        if (s.empty())
        {
            env->out() << "[w] no gate layout in store" << std::endl;
            reset_flags();
            return;
        }

        if (library == 0u)
        {
            //                // QCA-ONE only allows non-shifted layouts
            //                if (fgl->is_vertically_shifted())
            //                {
            //                    env->out() << "[e] non-shifted layouts are required for this library" << std::endl;
            //                    reset_flags();
            //                    return;
            //                }

            const auto apply_qca_one = [](auto&& lyt_ptr)
            {
                return std::make_shared<fiction::qca_cell_clk_lyt>(
                    fiction::apply_gate_library<fiction::qca_cell_clk_lyt, fiction::qca_one_library>(*lyt_ptr));
            };

            try
            {
                store<fiction::cell_layout_t>().extend() = std::visit(apply_qca_one, s.current());
            }
            catch (const fiction::unsupported_gate_type_exception<fiction::offset::ucoord_t>& e)
            {
                std::cout << fmt::format("[e] unsupported gate type at tile position {}", e.where()) << std::endl;
            }
            catch (const fiction::unsupported_gate_orientation_exception<fiction::offset::ucoord_t,
                                                                         fiction::port_position>& e)
            {
                std::cout << fmt::format("[e] unsupported gate orientation at tile position {} with ports {}",
                                         e.where(), e.which_ports())
                          << std::endl;
            }
        }
        //            else if (library == 1u)
        //            {
        //                // ToPoliNano only allows vertically shifted layouts
        //                if (!fgl->is_vertically_shifted())
        //                {
        //                    env->out() << "[e] vertically shifted layouts are required for this library" << std::endl;
        //                    reset_flags();
        //                    return;
        //                }
        //                // ToPoliNano only allows layouts clocked with a ToPoliNano clocking
        //                if (!fgl->is_clocking("TOPOLINANO3") && !fgl->is_clocking("TOPOLINANO4"))
        //                {
        //                    env->out() << "[e] a ToPoliNano clocking is required for this library" << std::endl;
        //                    reset_flags();
        //                    return;
        //                }
        //
        //                lib = std::make_shared<topolinano_library>(fgl);
        //            }
        else if (library == 2u)
        {
            const auto apply_sidb_bestagon = [](auto&& lyt_ptr) -> std::optional<fiction::sidb_cell_clk_lyt_ptr>
            {
                using Lyt = typename std::decay_t<decltype(lyt_ptr)>::element_type;

                if constexpr (fiction::is_hexagonal_layout_v<Lyt>)
                {
                    if constexpr (fiction::has_pointy_top_hex_orientation_v<Lyt>)
                    {
                        return std::make_shared<fiction::sidb_cell_clk_lyt>(
                            fiction::apply_gate_library<fiction::sidb_cell_clk_lyt, fiction::sidb_bestagon_library>(
                                *lyt_ptr));
                    }
                    else
                    {
                        std::cout << "[e] hexagonal orientation must be pointy-top" << std::endl;
                    }
                }
                else
                {
                    std::cout << "[e] layout topology must be hexagonal" << std::endl;
                }

                return std::nullopt;
            };

            try
            {
                if (const auto cell_lyt = std::visit(apply_sidb_bestagon, s.current()); cell_lyt.has_value())
                {
                    store<fiction::cell_layout_t>().extend() = *cell_lyt;
                }
            }
            catch (const fiction::unsupported_gate_type_exception<fiction::offset::ucoord_t>& e)
            {
                std::cout << fmt::format("[e] unsupported gate type at tile position {}", e.where()) << std::endl;
            }
            catch (const fiction::unsupported_gate_orientation_exception<fiction::offset::ucoord_t,
                                                                         fiction::port_position>& e)
            {
                std::cout << fmt::format("[e] unsupported gate orientation at tile position {} with ports {}",
                                         e.where(), e.which_ports())
                          << std::endl;
            }
            catch (const fiction::unsupported_gate_orientation_exception<fiction::offset::ucoord_t,
                                                                         fiction::port_direction>& e)
            {
                std::cout << fmt::format("[e] unsupported gate orientation at tile position {} with port directions {}",
                                         e.where(), e.which_ports())
                          << std::endl;
            }
        }
        // more libraries go here
        else
        {
            env->out() << "[e] identifier " << library << " does not refer to a supported gate library" << std::endl;
            reset_flags();
            return;
        }

        //        try
        //        {
        //            auto fcl = std::make_shared<fcn_cell_layout>(std::move(lib));
        //
        //            // store new layout
        //            store<fcn_cell_layout_ptr>().extend() = std::move(fcl);
        //        }
        //        catch (...)
        //        {
        //            env->out() << "[e] mapping " << s.current()->get_name() << " to cell-level using the " << lib_name
        //                       << " library was not successful" << std::endl;
        //            reset_flags();
        //            return;
        //        }

        reset_flags();
    }

  private:
    /**
     * Identifier of gate library to use.
     */
    unsigned library = 0u;

    /**
     * Reset all flags. Necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        library = 0u;
    }
};

ALICE_ADD_COMMAND(cell, "Technology")

}  // namespace alice

#endif  // FICTION_CMD_CELL_HPP