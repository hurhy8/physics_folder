#include <math.h>
#include <cmath>
#include <numbers>
#include <vector>
#include "polygon.h"
#include <algorithm>
#include <stdexcept>
#include <cfenv>
#include <cmath>
#include <iostream>
#include <cstdlib>

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;



void enable_fpe() {
    unsigned int current;
    _controlfp_s(&current, 0, 0);        // get current control word
    _controlfp_s(&current, 0, _MCW_EM);  // clear all exception masks
    _controlfp_s(&current, ~_EM_INVALID & ~_EM_ZERODIVIDE & ~_EM_OVERFLOW, _MCW_EM); 
    // Enable invalid, divide-by-zero, overflow traps
}
PYBIND11_MODULE(physics, m) {
   
    m.doc() = "2D physics engine";
    py::class_<Shape, std::shared_ptr<Shape>>(m, "Shape"),
    py::class_<Polygon, Shape, std::shared_ptr<Polygon>>(m, "Polygon")
    .def(py::init<int, double, double, double, int, double, double, double>(),
         py::arg("vertices") = 4,
         py::arg("vx") = 0,
         py::arg("center_x") = 400,
         py::arg("center_y") = 0,
         py::arg("length") = 110,
         py::arg("omega") = -3,
         py::arg("angle") = 0,
         py::arg("mass") = 25)
    .def("initialize_self_vector", &Polygon::initialize_self_vector)
        .def("apply_motion", &Polygon::apply_motion)
        .def("clamp_motion", &Polygon::clamp_motion)
        .def("wall", &Polygon::wall)
        .def("AABB_overlap", &Polygon::aabb_overlap)
        .def("update_world_vertices", &Polygon::update_world_vertices)
        .def("initialize_self_vector", &Polygon::initialize_self_vector)
        .def_readwrite("vx", &Polygon::vx)
        .def_readwrite("vy", &Polygon::vy)
        .def_readwrite("center_x", &Polygon::center_x)
        .def_readwrite("center_y", &Polygon::center_y)
        .def_readwrite("omega", &Polygon::omega)
        .def_readwrite("angle", &Polygon::angle)
        .def_readwrite("gravity", &Polygon::gravity)
        .def_readwrite("allow_motion", &Polygon::allow_motion)
        
        .def_readwrite("sleep", &Polygon::sleep)
        .def_readwrite("draw_animation", &Polygon::draw_animation)
        .def_readwrite("collided", &Polygon::collided)

        .def_readwrite("remove", &Polygon::remove)
        .def_readwrite("name", &Polygon::name)
        .def_readonly("aabb", &Polygon::aabb)
        .def_readonly("num_times_sleepy", &Polygon::num_times_sleepy)
        .def_readonly("length", &Polygon::length)
        .def_readwrite("world_vertices", &Polygon::world_vertices);
        
        py::class_<CompoundShape, Shape, std::shared_ptr<CompoundShape>>(m, "CompoundShape")
        .def(py::init<std::vector<std::shared_ptr<Shape>>, int, double, double, double>(),
             py::arg("children"),
             py::arg("vertices") = 4,
             py::arg("vx") = 0,
             py::arg("omega") = -3,
             py::arg("angle") = 0)
            
            
    
            // Expose data members (optional)
            .def_readwrite("child_polygons", &CompoundShape::child_polygons)
            .def_readwrite("vx", &CompoundShape::vx)
            .def_readwrite("vy", &CompoundShape::vy)
            .def_readwrite("center_x", &CompoundShape::center_x)
            .def_readwrite("center_y", &CompoundShape::center_y)
            .def_readwrite("omega", &CompoundShape::omega)
            .def_readwrite("angle", &CompoundShape::angle)
            .def_readwrite("gravity", &CompoundShape::gravity)
            .def_readwrite("allow_motion", &CompoundShape::allow_motion)
            .def("update_world_vertices", &CompoundShape::update_world_vertices)
            .def_readwrite("sleep", &CompoundShape::sleep)
            .def_readwrite("draw_animation", &CompoundShape::draw_animation)
            .def_readwrite("collided", &CompoundShape::collided)
    
            .def_readwrite("remove", &CompoundShape::remove)
            .def_readwrite("name", &CompoundShape::name)
            .def_readonly("aabb", &CompoundShape::aabb)
            .def_readonly("num_times_sleepy", &CompoundShape::num_times_sleepy)
            .def_readonly("length", &CompoundShape::length)
            .def_readwrite("world_vertices", &CompoundShape::world_vertices);
        py::class_<PolygonManager>(m, "PolygonManager")
            .def(py::init<std::shared_ptr<PolygonCollisions>>())
            .def("add_polygon", &PolygonManager::add_polygon)
            .def("remove_polygon", &PolygonManager::remove_polygon)
            .def("step", &PolygonManager::step);
    py::class_<PolygonCollisions, std::shared_ptr<PolygonCollisions>>(m, "PolygonCollisions")
        .def(py::init<>());
    py::class_<Vec2>(m, "Vec2")
        .def(py::init<double, double>()) 
        .def_readwrite("x", &Vec2::x)
        .def_readwrite("y", &Vec2::y);
    py::class_<AABB>(m, "AABB")
        .def(py::init<double, double, double, double>()) 
        .def_readwrite("min_x", &AABB::min_x)
        .def_readwrite("max_x", &AABB::max_x)
        .def_readwrite("min_y", &AABB::min_y)
        .def_readwrite("max_y", &AABB::max_y);
    py::class_<Rectangle, Polygon, std::shared_ptr<Rectangle>>(m, "Rectangle")
        .def(py::init<int, double, double, double, int, int, double, double, double>(),
            py::arg("vertices") = 4,
            py::arg("vx") = 0,
            py::arg("center_x") = 400,
            py::arg("center_y") = 0,
            py::arg("length") = 110,
            py::arg("width") = 70,
            py::arg("omega") = -3,
            py::arg("angle") = 0,  
            py::arg("mass") = 25
        );
    py::class_<IsoscelesTriangle, Polygon, std::shared_ptr<IsoscelesTriangle>>(m, "IsoscelesTriangle")
    .def(py::init<int, double, double, double, int, int, double, double, double>(),
            py::arg("vertices") = 4,
            py::arg("vx") = 0,
            py::arg("center_x") = 400,
            py::arg("center_y") = 0,
            py::arg("equal_side") = 110,
            py::arg("unique_side") = 70,
            py::arg("omega") = -3,
            py::arg("angle") = 0,  
            py::arg("mass") = 25
        )
        
        .def_readwrite("width", &IsoscelesTriangle::width);
        
}
