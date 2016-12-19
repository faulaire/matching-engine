#include <unordered_map>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <Engine_Order.h>
#include <Engine_Instrument.h>

namespace py = pybind11;
using namespace pybind11::literals;

using namespace exchange::engine;

class InstrumentManager : public exchange::engine::InstrumentManager<Order>
{
public:

    InstrumentManager(const std::string & sDBPath):
        exchange::engine::InstrumentManager<Order>(sDBPath)
    {}

    bool LoadInstruments()
    {
        auto instr_handler = [this](const auto & instr)
        {
            m_Instruments.emplace(instr.GetProductId(), instr);
        };

        return Load(instr_handler);
    }

    bool WriteInstrument(const Instrument<Order> & rInstrument)
    {
        auto key_extractor = [](const Instrument<Order> & Instrument) -> const std::string &
        {
            return Instrument.GetName();
        };

        return Write(rInstrument, key_extractor);
    }

    std::unordered_map<std::uint32_t, Instrument<Order>> m_Instruments;
};

void bind_InstrumentManager(py::module &m)
{
    using InstrumentType = Instrument<Order>;

    py::class_<Price>(m, "Price")
            .def(py::init<>())
            .def(py::init<std::uint32_t>())
            .def("__int__", [](Price & p){ return static_cast<uint32_t>(p); });

    py::implicitly_convertible<py::int_, Price>();

    py::class_<InstrumentType>(m, "Instrument")
            .def(py::init<>())
            .def(py::init<const std::string &, const std::string &, const std::string &, int, Price>())
            .def("GetName", &InstrumentType::GetName)
            .def("GetCurrency", &InstrumentType::GetCurrency)
            .def("GetIsin", &InstrumentType::GetIsin)
            .def("GetClosePrice", &InstrumentType::GetClosePrice)
            .def("GetProductId", &InstrumentType::GetProductId)
            .def("SetClosePrice", &InstrumentType::SetClosePrice)
            .def("__str__", [](InstrumentType & rInstr){ std::ostringstream oss; oss << rInstr; return oss.str(); } );

    py::bind_map<std::unordered_map<std::uint32_t, InstrumentType>>(m, "UnorderedMapProductIDToInstrument");

    py::class_<::InstrumentManager>(m, "InstrumentManager")
            .def(py::init<const std::string &>())
            .def("LoadInstruments", &::InstrumentManager::LoadInstruments)
            .def("WriteInstrument", &::InstrumentManager::WriteInstrument)
            .def_readonly("instruments", &::InstrumentManager::m_Instruments);
}

PYBIND11_PLUGIN(instrument_editor)
{
        py::module m("instrument_editor", "Instrument Editor for the matching engine");
        /**/
        bind_InstrumentManager(m);

        return m.ptr();
}