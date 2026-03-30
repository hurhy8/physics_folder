#include "polygon.h"

OptionalFractureVector PolygonManager::step(){
    OptionalFractureVector components;
    try {
        
        std::vector<std::shared_ptr<Shape>> no_joint_poly;
        for (auto&polygon:polygons){
            
        }
        
        for (auto&polygon:polygons){
            CompoundShape* compound = dynamic_cast<CompoundShape*>(polygon.get());
            if (compound){
                if (polygon->allow_motion){
                    compound->update_world_vertices();
                    compound->update_aabb();
                    for (auto child:compound->child_polygons){
                        child->vx = 0;
                        child->vy = 0;
                        child->omega = 0;

                    }
                    compound->apply_force(dt);
                    std::vector<Vec2> vert = polygon->world_vertices;
                    if (incrementer % 100 == 0){
                        collision_class->impulse_cache.clear();
                    }
                    if (vert.empty()){
                        std::cout << "world vert is empty!";
                    }
                    if (!polygon) {
                        std::cerr << "Warning: Null polygon detected, skipping.\n";
                        continue;
                    }

                    
                        
                    compound->clamp_motion();
                    if (pendulum_class->joints.size() == 0){
                        compound->put_to_sleep();
                    }
                    
                    //polygon->wall(Vec2(0, 0), Vec2(1700 - 1, 0));
                    compound->boundary_wall(Vec2(0, 0), Vec2(-17, 0), [](double y, double boundary) { return y < boundary; }, BoundaryType::Left);
                                
                                

                    // Bottom wall (from left to right)
                    
                    compound->boundary_wall(Vec2(0, 9), Vec2(17, 9), [](double y, double boundary) { return y > boundary; }, BoundaryType::Right);

                    // Left wall (from top to bottom)
                    
                    //polygon->wall(Vec2(0, 0), Vec2(0, 900 - 2));
                    //compound->boundary_wall(Vec2(0, 0), Vec2(0, 900 - 2), [](double x, double boundary) { return x < boundary; }, BoundaryType::Left);
                    // Right wall (from top to bottom)
                    
                    //polygon->wall(Vec2(1700 - 1, 0), Vec2(1700-1, 900 - 1));
                    compound->boundary_wall(Vec2(17, 0), Vec2(17, 9), [](double x, double boundary) { return x > boundary; }, BoundaryType::Right);
                
                }
                else{

                }
                
            }
            else{

                Polygon* poly = dynamic_cast<Polygon*>(polygon.get());
                if (poly){
                    if (poly->allow_motion){
                        poly->update_world_vertices();
                        poly->update_aabb();
                        poly->apply_force(dt);
                        std::vector<Vec2> vert = polygon->world_vertices;
                        if (incrementer % 100 == 0){
                            collision_class->impulse_cache.clear();
                        }
                        if (vert.empty()){
                            std::cout << "world vert is empty!";
                        }
                        if (!polygon) {
                            std::cerr << "Warning: Null polygon detected, skipping.\n";
                            continue;
                        }
     
                        
                            
                        poly->clamp_motion();
                        if (pendulum_class->joints.size() == 0){
                            poly->put_to_sleep();
                        }
                        //polygon->wall(Vec2(0, 0), Vec2(1700 - 1, 0));
                       // poly->boundary_wall(Vec2(0, 0), Vec2(1700 - 1, 0), [](double y, double boundary) { return y < boundary; }, BoundaryType::Left);
                        
                        
    
                        // Bottom wall (from left to right)
                        poly->boundary_wall(Vec2(0, 9), Vec2(17, 9), [](double y, double boundary) { return y > boundary; }, BoundaryType::Right);
                        
    
                        // Left wall (from top to bottom)
                        
                        //polygon->wall(Vec2(0, 0), Vec2(0, 900 - 2));
                        //poly->boundary_wall(
                     //       Vec2(0, 9),
                      //      Vec2(0, 0),
                     //       [](double x, double boundary) { return x < boundary; },
                       //     BoundaryType::Left
                        //);
                        // Right wall (from top to bottom)
                        
                        //polygon->wall(Vec2(1700 - 1, 0), Vec2(1700-1, 900 - 1));
                        
                        poly->boundary_wall(Vec2(17, 0), Vec2(17, 9), [](double x, double boundary) { return x > boundary; }, BoundaryType::Right);
                        
                    }
                    else{

                    }
                    
                }
            }
        }
        components = collision_class->polygon_collisions(polygons);
        pendulum_class->iterative_solver();
        for (auto&polygon:polygons){
            CompoundShape* compound = dynamic_cast<CompoundShape*>(polygon.get());
            if (compound){
                compound->apply_motion(dt);
            }
            else{

                Polygon* poly = dynamic_cast<Polygon*>(polygon.get());
                if (poly){
                    poly->apply_motion(dt);
                    
                    }
                
            }
        }
        
        ++incrementer;
        if (components){
            return components.value();
        }
        return std::nullopt;
        
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
        return std::nullopt;
}
}