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




PYBIND11_MODULE(physics, m) {
   
    m.doc() = "2D physics engine";
    py::class_<Shape, std::shared_ptr<Shape>>(m, "Shape"),
    py::class_<Polygon, Shape, std::shared_ptr<Polygon>>(m, "Polygon")
    .def(py::init<int, double, double, double, double, double, double, double, bool>(),
         py::arg("vertices") = 4,
         py::arg("vx") = 0,
         py::arg("center_x") = 400,
         py::arg("center_y") = 0,
         py::arg("length") = 110,
         py::arg("omega") = -3,
         py::arg("angle") = 0,
         py::arg("mass") = 25,
         py::arg("static") = false)
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
        .def_readonly("ID", &Polygon::id)
        .def_readwrite("angle", &Polygon::angle)
        .def_readwrite("gravity", &Polygon::gravity)
        .def_readwrite("allow_motion", &Polygon::allow_motion)
        .def_readwrite("local_center_x", &Polygon::local_center_x)
        .def_readwrite("local_center_y", &Polygon::local_center_y)
        .def_readwrite("sleep", &Polygon::sleep)
        .def_readwrite("draw_animation", &Polygon::draw_animation)
        .def_readwrite("collided", &Polygon::collided)
        .def_readwrite("ignore_collisions", &Polygon::ignore_collisions)
        .def_readwrite("inertia", &Polygon::inertia)
        .def_readwrite("remove", &Polygon::remove)
        .def_readwrite("name", &Polygon::name)
        .def_readonly("aabb", &Polygon::aabb)
        .def_readwrite("num_times_sleepy", &Polygon::num_times_sleepy)
        .def_readonly("length", &Polygon::length)
        .def_readwrite("world_vertices", &Polygon::world_vertices)
        .def_readwrite("static", &Polygon::static_body);
        
        py::class_<CompoundShape, Shape, std::shared_ptr<CompoundShape>>(m, "CompoundShape")
        .def(py::init<std::vector<std::shared_ptr<Shape>>, int, double, double, double, bool>(),
             py::arg("children"),
             py::arg("vertices") = 4,
             py::arg("vx") = 0,
             py::arg("omega") = -3,
             py::arg("angle") = 0,
             py::arg("static") = false)
            
            
    
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
            .def_readwrite("mass", &CompoundShape::mass)
            .def_readwrite("inertia", &CompoundShape::inertia)
            .def_readonly("ID", &CompoundShape::id)
            .def_readwrite("remove", &CompoundShape::remove)
            .def_readwrite("name", &CompoundShape::name)
            .def_readonly("aabb", &CompoundShape::aabb)
            .def_readwrite("num_times_sleepy", &CompoundShape::num_times_sleepy)
            .def_readonly("length", &CompoundShape::length)
            .def_readwrite("world_vertices", &CompoundShape::world_vertices);

            py::class_<PolygonManager, std::shared_ptr<PolygonManager>>(m, "PolygonManager")
            .def(py::init<
                std::shared_ptr<PolygonCollisions>,
                std::shared_ptr<PendulumSolver>
            >(),
                py::arg("collision_system"),
                py::arg("pendulum_solver")
            )
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
        .def(py::init<int, double, double, double, double, double, double, double, double, bool>(),
            py::arg("vertices") = 4,
            py::arg("vx") = 0,
            py::arg("center_x") = 400,
            py::arg("center_y") = 0,
            py::arg("length") = 110,
            py::arg("width") = 70,
            py::arg("omega") = -3,
            py::arg("angle") = 0,  
            py::arg("mass") = 25,
            py::arg("static") = false
        )
        
        .def_readwrite("width", &Rectangle::width)
        .def_readwrite("mass", &Rectangle::mass)
        .def_readonly("ID", &Rectangle::id)
        .def_readwrite("local_center_x", &Rectangle::local_center_x)
        .def_readwrite("local_center_y", &Rectangle::local_center_y);
    py::class_<RightTriangle, Polygon, std::shared_ptr<RightTriangle>>(m, "RightTriangle")
    .def(py::init<double, double, double, double, double, double, double, double>(),
            
            py::arg("vx") = 0,
            py::arg("center_x") = 40,
            py::arg("center_y") = 0,
            py::arg("width") = 11,
            py::arg("length") = 7,
            py::arg("omega") = -3,
            py::arg("angle") = 0,  
            py::arg("mass") = 25
            
        )
    
        .def_readwrite("width", &RightTriangle::width);
        py::class_<Joint, std::shared_ptr<Joint>>(m, "Joint");
        py::class_<PendulumJoint, Joint, std::shared_ptr<PendulumJoint>>(m, "PendulumJoint")
    .def(py::init<Vec2, Vec2, std::shared_ptr<Shape>>())
    .def("update_local_anchor", &PendulumJoint::update_local_anchor)
    .def_readwrite("rigid_body", &PendulumJoint::rigid_body)
    .def_readwrite("body_anchor", &PendulumJoint::body_anchor)
    .def_readwrite("world_anchor", &PendulumJoint::world_anchor)
    .def_readwrite("rope_length", &PendulumJoint::rope_length);

py::class_<DynamicJoint, Joint, std::shared_ptr<DynamicJoint>>(m, "DynamicJoint")
    .def(py::init<Vec2, Vec2, std::shared_ptr<Shape>, std::shared_ptr<Shape>>())
    .def("update_local_anchor", &DynamicJoint::update_local_anchor)
    .def_readwrite("rigid_body1", &DynamicJoint::rigid_body1)
    .def_readwrite("rigid_body2", &DynamicJoint::rigid_body2)
    .def_readwrite("body_anchor1", &DynamicJoint::body_anchor1)
    .def_readwrite("body_anchor2", &DynamicJoint::body_anchor2)
    .def_readwrite("rope_length", &DynamicJoint::rope_length);


py::class_<PivotJoint, Joint, std::shared_ptr<PivotJoint>>(m, "PivotJoint")
    .def(py::init<Vec2, Vec2, std::shared_ptr<Shape>, std::shared_ptr<Shape>>())
    .def("update_local_anchor", &PivotJoint::update_local_anchor)
    .def_readwrite("rigid_body1", &PivotJoint::rigid_body1)
    .def_readwrite("rigid_body2", &PivotJoint::rigid_body2)
    .def_readwrite("body_anchor1", &PivotJoint::body_anchor1)
    .def_readwrite("body_anchor2", &PivotJoint::body_anchor2);



py::class_<SoftJoint, Joint, std::shared_ptr<SoftJoint>>(m, "SoftJoint")
    .def(py::init<Vec2, Vec2, std::shared_ptr<Shape>, std::shared_ptr<Shape>, double>())
    .def("update_local_anchor", &SoftJoint::update_local_anchor)
    .def_readwrite("rigid_body1", &SoftJoint::rigid_body1)
    .def_readwrite("rigid_body2", &SoftJoint::rigid_body2)
    .def_readwrite("body_anchor1", &SoftJoint::body_anchor1)
    .def_readwrite("body_anchor2", &SoftJoint::body_anchor2)
    .def_readwrite("rope_length", &SoftJoint::rope_length)
    .def_readwrite("k", &SoftJoint::k);

    py::class_<PendulumSolver, std::shared_ptr<PendulumSolver>>(m, "PendulumSolver")
    .def(py::init<>())
    .def_readwrite("joints", &PendulumSolver::joints)
    .def("add_joint",
        [](PendulumSolver& self, std::shared_ptr<Joint> joint) {
            self.joints.push_back(joint);
        }
    )

    .def("solve", &PendulumSolver::iterative_solver)
    .def("remove_rigid_body", &PendulumSolver::remove_rigid_body);
}
