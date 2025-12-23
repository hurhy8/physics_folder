import pygame
import math
import random
import physics

pygame.init()
WIDTH, HEIGHT = 1700, 900
screen = pygame.display.set_mode((WIDTH, HEIGHT))
clock = pygame.time.Clock()

# A simple function to create a random polygon
collision = physics.PolygonCollisions()
man = physics.PolygonManager(collision)

# Store all compound shapes
shapes = []

# Main loop
running = True
polygon = physics.Polygon(4, length=200, mass=30, center_x=300, center_y=300)
polygon.initialize_self_vector()
man.add_polygon(polygon)

#man.add_polygon(polygon2)
shapes.append(polygon)
def create_star(mx, my):
    children2 = []
    L = 160
    R = L * 0.3

    for i in range(11):
        ang = math.radians(i * 360/11)

        cx = mx + R * math.cos(ang)
        cy = my + R * math.sin(ang)
        poly =    physics.Polygon(
                vertices=3,
                vx=0,
                center_x=cx,
                center_y=cy,
                length=L,
                mass=20,
                angle=ang  # rotate triangle to point outward
            )
        poly.initialize_self_vector()
        children2.append(
            poly
        )
        




    shape2 = physics.CompoundShape(children2, 
                                vertices=sum(len(c.world_vertices) for c in children2),
                                vx=00, omega=0, angle=0)
    shape2.angle = (math.pi/1.5 * (len(shape2.world_vertices) - 2))/(2*len(shape2.world_vertices))
    return shape2

heart_shapes = []
def create_heart_hitbox(mx, my):
    children2 = []
    L = 160

    square1 =    physics.Polygon(
            vertices=8,
            vx=0,
            center_x=mx,
            center_y=my,
            length=int(L/2),
            mass=15,
            angle=0  # rotate triangle to point outward
        )
    square1.initialize_self_vector()
    children2.append(
        square1
    )
    square2 =    physics.Polygon(
        vertices=8,
        vx=0,
        center_x=mx-110,
        center_y=my+110,
        length=int(L/2),
        mass=15,
        angle=0  # rotate triangle to point outward
    )
    square2.initialize_self_vector()

    triangle =    physics.Polygon(
    vertices=3,
    vx=0,
    center_x=mx+35,
    center_y=my+145,
    length=L,
    mass=15,
    angle=math.radians(-195)  # rotate triangle to point outward
)
    triangle.initialize_self_vector()
    children2.append(
    triangle
)

    triangle.initialize_self_vector()
    children2.append(
    square2
)
     
        




    shape2 = physics.CompoundShape(children2, 
                                vertices=sum(len(c.world_vertices) for c in children2),
                                vx=3000, omega=20, angle=0)
    shape2.angle = math.radians(45)
    return shape2

def create_oval_hitbox(mx, my):
    children2 = []
    L = 240

    square1 =    physics.Polygon(
            vertices=16,
            vx=0,
            center_x=mx,
            center_y=my,
            length=int(L/4),
            mass=15,
            angle=0  # rotate triangle to point outward
        )
    square1.initialize_self_vector()
    children2.append(
        square1
    )
    square2 =    physics.Polygon(
        vertices=16,
        vx=0,
        center_x=mx-15,
        center_y=my+15,
        length=int(L/4),
        mass=15,
        angle=0  # rotate triangle to point outward
    )
    square2.initialize_self_vector()
    children2.append(
        square2
    )
 
     
        




    shape2 = physics.CompoundShape(children2, 
                                vertices=sum(len(c.world_vertices) for c in children2),
                                vx=3000, omega=20, angle=0)
    shape2.angle = math.radians(45)
    return shape2

heart_img = pygame.image.load("heart2.png").convert_alpha()
oval_img = pygame.image.load("oval.png").convert_alpha()
ovals = []




polygon_main =    physics.Rectangle(
    vertices=4,
    vx=0,
    center_x=700,
    center_y=400,
    length=int(1000/4),
    mass=15,
    angle=0  # rotate triangle to point outward
)
polygon_main.name = "main_polygon"
polygon_main.remove = False
polygon_main.initialize_self_vector()
shapes.append(polygon_main)
man.add_polygon(polygon_main)

initial_pos = ()
def update_velocity_drag(polygon: physics.Polygon, initial_pos, mouse_release_pos):
    dx = (initial_pos[0] - mouse_release_pos[0])* 8

    dy = (initial_pos[1] - mouse_release_pos[1]) * 8
    
    #print(initial_pos)
    
    
    polygon.vx = dx
    polygon.vy = dy
def draw_trajectory(initial_pos, mouse_pos_x, mouse_pos_y, screen):
    trajectory = []
    

    #print(velocity, velocity_x)

    projected_velocity_x = (initial_pos[0] - mouse_pos_x) * 8
    projected_velocity_y = (initial_pos[1] - mouse_pos_y) * 8
    flight_time = 0.15
    for i in range(1, 300):
        
        t = i/240
        x = mouse_pos_x + projected_velocity_x * t
        y = mouse_pos_y + projected_velocity_y * t + 0.5 * (9.8 * 100) * (t)**2
        
        trajectory.append((x, y))
    for i, point in enumerate(trajectory):
                
                if i + 1 >= len(trajectory):
                    return
                pygame.draw.circle(screen, (255, 255, 255), (int(point[0]), int(point[1])), 5)
while running:
    
    screen.fill((0, 0, 0))

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.MOUSEBUTTONDOWN:
            mx, my = pygame.mouse.get_pos()
            rect =    physics.Rectangle(
                vertices=4,
                vx=0,
                center_x=random.randint(500, 1500),
                center_y=random.randint(200, 700),
                length=int(120),
                mass=15,
                angle=0  # rotate triangle to point outward
            )

            rect.allow_motion = False
            rect.initialize_self_vector()
            shapes.append(rect)
            man.add_polygon(rect)
    
            pos = pygame.mouse.get_pos()
            #new_polygon = physics.Polygon(self.vertices, length=self.length, mass=self.mass, center_x=pos[0], center_y=pos[1], omega=0, vx=0)
            #new_polygon.initialize_self_vector()
            #man.add_polygon(new_polygon)
            #self.polygons.append(new_polygon)
            #new_polygon.angle = (math.pi * (self.vertices - 2))/(2*self.vertices)
            for polygon in shapes:
                polygon.allow_motion = False
            
            initial_pos = pygame.mouse.get_pos()
            

        if event.type == pygame.MOUSEBUTTONUP:
            
                
            mouse_release_pos = pygame.mouse.get_pos()
            update_velocity_drag(polygon_main, initial_pos, mouse_release_pos)

            index_launch = 0
            
            polygon_main.gravity = 9.8
            y1 = 0
            
    buttons = pygame.mouse.get_pressed()
    if buttons[0]:
        pos = pygame.mouse.get_pos()
        velocity_threshold = 150

    
        
        polygon_main.center_x = pos[0]
    
        polygon_main.center_y = pos[1]
        
        polygon_main.sleep = False


    for heart in heart_shapes:
        all_vertices = [v for child in heart.child_polygons for v in child.world_vertices]
        min_x = min(v.x for v in all_vertices)
        max_x = max(v.x for v in all_vertices)
        min_y = min(v.y for v in all_vertices)
        max_y = max(v.y for v in all_vertices)

        width = max_x - min_x
        height = max_y - min_y
        center_x = heart.center_x  # your CompoundShape center
        center_y = heart.center_y

        # Scale first
        

        # Rotate after scaling
        desired_width = 350
        desired_height = 350

        # Scale to the size you want
        heart_img_scaled = pygame.transform.scale(heart_img, (desired_width, desired_height))
        heart_img_rotated = pygame.transform.rotate(heart_img_scaled, -math.degrees(heart.angle  + math.radians(-45)))

        # Center rotated rect
        rot_rect = heart_img_rotated.get_rect(center=(center_x, center_y))

        # Draw it
        screen.blit(heart_img_rotated, rot_rect.topleft)
    # Update all shapes
   
    
    for oval in ovals:
            
            min_x = min(v.x for v in oval.world_vertices)
            max_x = max(v.x for v in oval.world_vertices)
            min_y = min(v.y for v in oval.world_vertices)
            max_y = max(v.y for v in oval.world_vertices)

            width = max_x - min_x
            height = max_y - min_y
            center_x = oval.center_x  # your CompoundShape center
            center_y = oval.center_y

            # Scale first
            

            # Rotate after scaling
            desired_width = 300
            desired_height = 300

            # Scale to the size you want
            oval_scaled = pygame.transform.scale(oval_img, (desired_width, desired_height))
            oval_rotated = pygame.transform.rotate(oval_scaled, -math.degrees(oval.angle  + math.radians(45)))

            # Center rotated rect
            rot_rect = oval_rotated.get_rect(center=(center_x, center_y))

            # Draw it
            screen.blit(oval_rotated, rot_rect.topleft)
    


    colors = [(0, 255, 0)]  # green, white, red

   
    
    #polygon2 = physics.Polygon(center_x=900, vx=1500)

    for i, shape in enumerate(shapes.copy()):
        
        if isinstance(shape, physics.CompoundShape):
            for j, child in enumerate(shape.child_polygons):
                points = [(v.x, v.y) for v in child.world_vertices]
                color = colors[j % len(colors)]
                pygame.draw.polygon(screen, color, points, width=1)
        else:
            points = [(v.x, v.y) for v in shape.world_vertices]
            if shape.collided:
                shape.allow_motion = True
            if shape.remove:
                print("LALALALLA")
                shapes.remove(shape)
                man.remove_polygon(shape)
               
            pygame.draw.polygon(screen, colors[0], points, width=1)
    fractured_pieces = man.step()
    if fractured_pieces:
        
        for fracture in fractured_pieces:
            
            frac1: physics.Rectangle = fracture[0][0]
            frac2: physics.Rectangle = fracture[0][1]
            frac1.initialize_self_vector()
            frac2.initialize_self_vector()
            to_remove = fracture[1]
            shapes.append(frac1)
            shapes.append(frac2)
            if to_remove in shapes:
                shapes.remove(to_remove)
                man.remove_polygon(to_remove)
    clock.tick(240)
    pygame.display.flip()

pygame.quit()