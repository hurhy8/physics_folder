#include "polygon.h"

OptionalFractureVector PolygonManager::step(){
    OptionalFractureVector components;
    try {

        for (auto&polygon:polygons){
            CompoundShape* compound = dynamic_cast<CompoundShape*>(polygon.get());
            if (compound){
                if (polygon->allow_motion){
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

                    compound->apply_motion(dt);
                        
                    compound->clamp_motion();
                    compound->put_to_sleep();
                    //polygon->wall(Vec2(0, 0), Vec2(1700 - 1, 0));
                    compound->boundary_wall(Vec2(0, 0), Vec2(1700 - 1, 0), [](double y, double boundary) { return y < boundary; }, BoundaryType::Left);
                    
                    

                    // Bottom wall (from left to right)
                    
                    compound->boundary_wall(Vec2(0, 900 - 1), Vec2(1700 - 1, 900 - 1), [](double y, double boundary) { return y > boundary; }, BoundaryType::Right);

                    // Left wall (from top to bottom)
                    
                    //polygon->wall(Vec2(0, 0), Vec2(0, 900 - 2));
                    //compound->boundary_wall(Vec2(0, 0), Vec2(0, 900 - 2), [](double x, double boundary) { return x < boundary; }, BoundaryType::Left);
                    // Right wall (from top to bottom)
                    
                    //polygon->wall(Vec2(1700 - 1, 0), Vec2(1700-1, 900 - 1));
                    compound->boundary_wall(Vec2(1700 - 1, 0), Vec2(1700-1, 900 - 1), [](double x, double boundary) { return x > boundary; }, BoundaryType::Right);
                
                }
                else{
                    compound->update_world_vertices();
                    compound->update_aabb();
                }
                
            }
            else{

                Polygon* poly = dynamic_cast<Polygon*>(polygon.get());
                if (poly){
                    if (poly->allow_motion){

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
     
                        poly->apply_motion(dt);
                            
                        poly->clamp_motion();
                        poly->put_to_sleep();
                        //polygon->wall(Vec2(0, 0), Vec2(1700 - 1, 0));
                        poly->boundary_wall(Vec2(0, 0), Vec2(1700 - 1, 0), [](double y, double boundary) { return y < boundary; }, BoundaryType::Left);
                        
                        
    
                        // Bottom wall (from left to right)
                        
                        poly->boundary_wall(Vec2(0, 900 - 1), Vec2(1700 - 1, 900 - 1), [](double y, double boundary) { return y > boundary; }, BoundaryType::Right);
    
                        // Left wall (from top to bottom)
                        
                        //polygon->wall(Vec2(0, 0), Vec2(0, 900 - 2));
                       // poly->boundary_wall(Vec2(0, 0), Vec2(0, 900 - 2), [](double x, double boundary) { return x < boundary; }, BoundaryType::Left);
                        // Right wall (from top to bottom)
                        
                        //polygon->wall(Vec2(1700 - 1, 0), Vec2(1700-1, 900 - 1));
                        poly->boundary_wall(Vec2(1700 - 1, 0), Vec2(1700-1, 900 - 1), [](double x, double boundary) { return x > boundary; }, BoundaryType::Right);
                    
                    }
                    else{
                        poly->update_world_vertices();
                        poly->update_aabb();
                    }
                    
                }
            }
        }
        components = collision_class->polygon_collisions(polygons);
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