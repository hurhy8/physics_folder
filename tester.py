import pygame
import math
import random
import sys
print(sys.path)
import physics
PPM = 100
pygame.init()
WIDTH, HEIGHT = 1700, 900
screen = pygame.display.set_mode((WIDTH, HEIGHT))
clock = pygame.time.Clock()

# A simple function to create a random polygon
collision = physics.PolygonCollisions()
pendulum_solver = physics.PendulumSolver()
man = physics.PolygonManager(collision, pendulum_solver)

# Store all compound shapes
shapes = []

# Main loop
running = True
polygon = physics.Polygon(4, length=200, mass=30, center_x=300, center_y=300)
polygon.initialize_self_vector()
#man.add_polygon(polygon)
font = pygame.font.SysFont(None, 32)
#man.add_polygon(polygon2)
shapes.append(polygon)
def create_car(mx, my):
    children2 = []
    body =    physics.Rectangle(
                        vertices=4,
                        vx=0,
                        center_x=mx/PPM,
                        center_y=my/PPM,
                        length=220/PPM,
                        width = 25/PPM,
                        mass=1,
                        angle=0,
                        static = False # rotate triangle to point outward
                    )

    wheel1 =    physics.Polygon(
                        vertices=30,
                        vx=0,
                        center_x=mx + 2.1,
                        center_y=my + 1.1,
                        length=2/PPM,
                        mass=16,
                        angle=0,
                        
                    )
    wheel2 =    physics.Polygon(
                        vertices=260,
                        vx=0,
                        center_x=mx - 2.1,
                        center_y=my-1.1,
                        length=0.007,
                        mass=16,
                        angle=0,
                        
                    )
    body.initialize_self_vector()
    wheel1.initialize_self_vector()
    wheel2.initialize_self_vector()
    children2.append(
        wheel1
    )

    
    shape2 = physics.CompoundShape(children2, 
                                vertices=sum(len(c.world_vertices) for c in children2),
                                vx=00, omega=15, angle=0)
    shape2.vx = 0

    shape2.vy = 0
    shape2.center_y=mx/PPM
    shape2.center_x=my/PPM

    return wheel1

def create_circle_enclosure(mx, my):
    children2 = []
    radius = 300 / PPM
    points = 30

    for i in range(points):
        ang = math.radians(i * 360 / points)
        local_x = radius * math.cos(ang)
        local_y = radius * math.sin(ang)

        poly = physics.Rectangle(
            vertices=4,
            vx=0,
            center_x=0,
            center_y=0,
            length=150/PPM,
            width=25/PPM,
            mass=1,
            angle=ang,
            static=False
        )
        poly.local_center_x = local_x
        poly.local_center_y = local_y
        poly.initialize_self_vector()
        children2.append(poly)
        poly.update_world_vertices()
    for n, c in enumerate(children2[:3]):
        print("RECT", n)
        for v in c.world_vertices:
            print(v.x, v.y)
    shape2 = physics.CompoundShape(
        children2,
        vertices=sum(len(c.world_vertices) for c in children2),
        vx=0,
        omega=0,
        angle=0,
        
    )
    


    shape2.center_x = mx/PPM
    shape2.center_y = my/PPM
    shapes.append(shape2)
    man.add_polygon(shape2)
    return shape2
def create_star(mx, my):
    children2 = []
    L = 160
    R = L * 0.3
   
    for i in range(4):
        ang = math.radians(i * 360/4)

        cx = mx/PPM + R/PPM * math.cos(ang)
        cy = my/PPM + R/PPM * math.sin(ang)
        poly =    physics.Polygon(
                vertices=3,
                vx=0,
                center_x=cx,
                center_y=cy,
                length=L/PPM,
                mass=16,
                angle=ang ,
                static = True # rotate triangle to point outward
            )
        poly.initialize_self_vector()
        children2.append(
            poly
        )
        




    shape2 = physics.CompoundShape(children2, 
                                vertices=sum(len(c.world_vertices) for c in children2),
                                vx=00, omega=15, angle=0)
    shape2.angle = (math.pi/1.5 * (len(shape2.world_vertices) - 2))/(2*len(shape2.world_vertices))
    shape2.center_y=300
    shape2.center_x=750
    shape2.vx = 0
    shape2.vy = 0
    for i in shape2.world_vertices:
        print(i.x, i.y)
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



"""
polygon_main = physics.Polygon(
        vertices=100,
        vx=0,
        center_x=-0/PPM,
        center_y=450/PPM,
        length=4/PPM,
        mass=25,
        omega=0,
        angle=math.radians(0),
        static=False
    )
joint = physics.PendulumJoint(
    physics.Vec2(polygon_main.length/2, polygon_main.length/2),
    physics.Vec2(400/PPM, 0),
    polygon_main
)
"""

polygon_main = create_car(500, 500)
create_circle_enclosure(800,300)
#pendulum_solver.add_joint(joint)

#polygon_main.initialize_self_vector()
polygon_main.name = "main_polygon"
polygon_main.remove = False

shapes.append(polygon_main)
man.add_polygon(polygon_main)

initial_pos = ()
def update_velocity_drag(polygon: physics.Polygon, initial_pos, mouse_release_pos):
    dx = ((initial_pos[0] - mouse_release_pos[0])/PPM)* 8 

    dy = ((initial_pos[1] - mouse_release_pos[1])/PPM) * 8
    
    #print(initial_pos)
    
    
    polygon.vx = dx
    polygon.vy = dy
def draw_trajectory(initial_pos, mouse_pos_x, mouse_pos_y, screen):
    trajectory = []
    

    #print(velocity, velocity_x)

    projected_velocity_x = ((initial_pos[0] - mouse_pos_x)/PPM) * 8
    projected_velocity_y = ((initial_pos[1] - mouse_pos_y)/PPM) * 8
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


def create_stick(cx, cy, width, length, mass=0.3):
    stick = physics.Rectangle(
        vertices=4,
        vx=0,
        center_x=cx,
        center_y=cy,
        length=int(length),
        width=int(width),
        mass=mass,
        omega=0,
        angle=math.radians(0),
        static=False
    )

    stick.initialize_self_vector()
    man.add_polygon(stick)
    shapes.append(stick)
    return stick


def create_weight(cx, cy, size, mass, angle):
    poly = physics.Polygon(
        vertices=4,
        vx=0,
        center_x=cx,
        center_y=cy,
        length=size,
        mass=mass,
        omega=0,
        angle=math.radians(angle),
        static=False
    )

    poly.initialize_self_vector()
    man.add_polygon(poly)
    shapes.append(poly)
    return poly


def attach(child, parent, anchor_child, anchor_parent):
    joint = physics.DynamicJoint(
        anchor_child,
        anchor_parent,
        child,
        parent
    )
    pendulum_solver.add_joint(joint)
def create_single(parent, offset_x, offset_y):
    
    cx = parent.center_x + offset_x
    cy = parent.center_y + offset_y
    w0 = create_weight(cx, cy + 100, 20, 38.5, -45)
    attach(w0, parent, physics.Vec2(0,0), physics.Vec2(-parent.width/2* 0.7355844,0))


def create_branch(parent, offset_x, offset_y, width, depth, masses, masses_stick):
    """
    Recursive branch builder.
    """


    cx = parent.center_x + offset_x
    cy = parent.center_y + offset_y
    if depth <= 0:

        return

    stick = create_stick(cx, cy, width, 20, masses_stick[0])

    attach(
        stick,
        parent,
        physics.Vec2(0, 0),
        physics.Vec2(offset_x, 0)
    )

    # left weight
    if depth == 1:
        if masses[0] == 7.9:
            w1 = create_weight(cx - width/2 * 0.2911392, cy + 100, 20, masses[0], -45)
            w2 = create_weight(cx + width/2, cy + 100, 20, masses[1], 45)
            attach(w1, stick, physics.Vec2(0,0), physics.Vec2(-width/2 * 0.2911392,0))
            print(masses, offset_x)
            # right weight
            
            attach(w2, stick, physics.Vec2(0,0), physics.Vec2(width/2,0))
        else:
            w1 = create_weight(cx - width/2 * 0.9915611, cy + 100, 20, masses[0], -45)
            w2 = create_weight(cx + width/2, cy + 100, 20, masses[1], 45)
            attach(w1, stick, physics.Vec2(0,0), physics.Vec2(-width/2 * 0.9915611,0))
            print(masses, offset_x)
            # right weight
            
            attach(w2, stick, physics.Vec2(0,0), physics.Vec2(width/2,0))


    # spawn child branches
    if depth == 2:
    
        create_branch(stick, -width/2 * 0.6326241, 150, width*0.6, depth-1, masses=(masses[0], masses[1]), masses_stick=(masses_stick[1],))
        create_branch(stick,  width/2, 150, width*0.6, depth-1, masses=(masses[2], masses[3]), masses_stick=(masses_stick[2],))


def create_mobile(mx, my):

    top = create_stick(WIDTH/2, my, 540, 20, 5.8)

    joint = physics.PendulumJoint(
        physics.Vec2(0, -top.length/2),
        physics.Vec2(WIDTH/2, 0),
        top
    )
    pendulum_solver.add_joint(joint)

    create_single(top, -top.width/2 *0.7355844, 150)
    create_branch(top,  top.width/2, 150, 150, depth=2, masses=(7.9, 2.3, 2.37, 2.35), masses_stick=(5.3, 3.9, 4.2))
def create_rope(mx, my):
    total_height_y =0
    original_length = 13
    
    
    num_rects = 45
    offset_x = 13.01  # x-axis offset
    top_mass = 1     # mass of the first movable segment
    bottom_mass = 0.01  # mass of the last segment
    mass = 0.1
    previous_rect = None

    for i in range(num_rects):
        # Compute mass for this segment
        #if i == 0:
            # mass = 1  # static anchor
        #else:
            # Linear interpolation from top_mass to bottom_mass
            #mass = top_mass * (bottom_mass / top_mass) ** ((i - 1) / (num_rects - 2))
        
        rect = physics.Rectangle(
            vertices=4,
            vx=0,
            center_x=mx,
            center_y=my + i * offset_x,
            length=int(original_length),
            width=10,
            mass=mass,
            omega=0,
            angle=math.radians(0),
            static = (i == 0)
        )
        rect.initialize_self_vector()
        man.add_polygon(rect)
        shapes.append(rect)

        if previous_rect is not None:
            joint = physics.DynamicJoint(
                physics.Vec2(0, rect.length/2),
                physics.Vec2(0, -rect.length/2),
                previous_rect,
                rect
            )
            pendulum_solver.add_joint(joint)

        previous_rect = rect



def create_bridge(mx, my):
    total_height_y =0
    original_length = 13
    
    
    num_rects = 45
    offset_x = 25.01  # x-axis offset
    top_mass = 1     # mass of the first movable segment
    bottom_mass = 0.01  # mass of the last segment
    mass = 0.1
    previous_rect = None

    for i in range(num_rects):
        # Compute mass for this segment
        #if i == 0:
            # mass = 1  # static anchor
        #else:
            # Linear interpolation from top_mass to bottom_mass
            #mass = top_mass * (bottom_mass / top_mass) ** ((i - 1) / (num_rects - 2))
        
        rect = physics.Rectangle(
            vertices=4,
            vx=0,
            center_x=mx + i * offset_x,
            center_y=my,
            length=int(10),
            width=25,
            mass=mass,
            omega=0,
            angle=math.radians(0),
            static = (i == 0 or i == num_rects - 1)
        )
        rect.initialize_self_vector()
        man.add_polygon(rect)
        shapes.append(rect)

        if previous_rect is not None:
            joint = physics.DynamicJoint(
                physics.Vec2(rect.width/2, 0),
                physics.Vec2(-rect.width/2, 0),
                previous_rect,
                rect
            )
            pendulum_solver.add_joint(joint)

        previous_rect = rect
total_width_x = 0
def create_stack(mx, my):
    global total_width_x
    total_height_y =0
    original_length = 70
    
    
    
        
    for i in range(1):
        for i in range(10):

            
            rect = physics.Polygon(
                vertices=4,
                vx=0,
                center_x=(WIDTH- ((original_length / 2) + total_width_x)-900)/PPM,
                center_y=(HEIGHT-(((original_length/2) + total_height_y)))/PPM,
                length=original_length/PPM,
                mass=0.01,
                omega=0,
                angle=math.radians(22.5))
            rect.initialize_self_vector()

            rect.name = "to_watch"
            edges = []
            for i in range(len(rect.world_vertices)):
                vertex: physics.Vec2
                edge = physics.Vec2(rect.world_vertices[(i+1) % len(rect.world_vertices)].x - rect.world_vertices[i].x, rect.world_vertices[(i+1) % len(rect.world_vertices)].y - rect.world_vertices[i].y)
                edge_length = math.sqrt(edge.x ** 2 + edge.y ** 2)
                edges.append(edge_length)
                print(edge_length)
            half_perimeter = sum(edges)/2
            s = half_perimeter
            height = (math.sqrt(s * ( s- edges[0]) * (s - edges[1]) * (s - edges[2]))*2)/ (edges[0])

            

            
            rect.angle = math.radians(22.5)

            total_height_y += original_length
            print("YOOO OMEGA", rect.omega)
            shapes.append(rect)
            man.add_polygon(rect)
        total_width_x += original_length
        total_height_y = 0
                

while running:

    screen.fill((0, 0, 0))

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:
                mx, my = pygame.mouse.get_pos()
                create_stack(mx, my)
                
                pos = pygame.mouse.get_pos()
                #new_polygon = physics.Polygon(self.vertices, length=self.length, mass=self.mass, center_x=pos[0], center_y=pos[1], omega=0, vx=0)
                #new_polygon.initialize_self_vector()
                #man.add_polygon(new_polygon)
                #self.polygons.append(new_polygon)
                #new_polygon.angle = (math.pi * (self.vertices - 2))/(2*self.vertices)
                #for polygon in shapes:
                #    polygon.allow_motion = False
                
                
                
            elif event.button == 3:
                initial_pos = pygame.mouse.get_pos()
        if event.type == pygame.MOUSEBUTTONUP:
            if event.button == 3:
            
                
                mouse_release_pos = pygame.mouse.get_pos()
                update_velocity_drag(polygon_main, initial_pos, mouse_release_pos)

                index_launch = 0
                
                #polygon_main.gravity = 9.8
                y1 = 0
            
    buttons = pygame.mouse.get_pressed()
    if buttons[2]:
        pos = pygame.mouse.get_pos()
        velocity_threshold = 150

    
        
        polygon_main.center_x = pos[0]/PPM
    
        polygon_main.center_y = pos[1]/PPM
        polygon_main.sleep = False
       # polygon_main.sleep = False


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
                pendulum_solver.remove_rigid_body(to_remove)
   
    current_fps = f"FPS: {(clock.get_fps()):.2f}"
    fps_surface = font.render(current_fps, True, (255, 255, 255))
    fps_pos = (600, 50)
    screen.blit(fps_surface, fps_pos)
    polygons_surface = font.render(f"amount of objects simulated: {len(shapes)}", True, (255, 255, 255))
    fps_pos = (800, 50)
    screen.blit(polygons_surface, fps_pos)
    #polygon2 = physics.Polygon(center_x=900, vx=1500)
    for joint in pendulum_solver.joints:
        
        if isinstance(joint, physics.PendulumJoint):
            joint: physics.PendulumJoint
            world_body_vec = joint.update_local_anchor()
            
            
            pygame.draw.line(screen, (255, 255, 255), (world_body_vec.x*PPM, world_body_vec.y*PPM), (joint.world_anchor.x*PPM,joint.world_anchor.y*PPM), 11)
        else:
            joint: physics.DynamicJoint
            world_body_vec = joint.update_local_anchor(joint.body_anchor1, joint.rigid_body1)
            world_body_vec2 = joint.update_local_anchor(joint.body_anchor2, joint.rigid_body2)
            
            pygame.draw.line(screen, (255, 255, 255), (world_body_vec.x, world_body_vec.y), (world_body_vec2.x,world_body_vec2.y), 11)

    for i, shape in enumerate(shapes.copy()):
        shape: physics.Polygon
        draw_x = shape.center_x * PPM 
        draw_y = shape.center_y * PPM
       
        if isinstance(shape, physics.CompoundShape):
            #print(shape.vx, shape.vy, shape.omega, "velocities")
            for j, child in enumerate(shape.child_polygons):
                points = [(v.x*PPM , v.y*PPM ) for v in child.world_vertices]
                color = colors[j % len(colors)]
                pygame.draw.polygon(screen, color, points, width=1)
        else:
            points = [(v.x * PPM, v.y * PPM) for v in shape.world_vertices]
            pygame.draw.line(screen, (123, 56, 3), (shape.center_x * PPM, shape.center_y * PPM), (shape.world_vertices[0].x*PPM,shape.world_vertices[0].y*PPM), 1)
            pygame.draw.polygon(screen, (255,0,0), points, 1)
            if shape.collided:
                shape.allow_motion = True
            if shape.remove:
                print("LALALALLA")
                shapes.remove(shape)
                man.remove_polygon(shape)
           # if shape.name == "main_polygon":
            #    print(shape.vx, shape.vy)
            

    clock.tick(240)
    pygame.display.flip()

pygame.quit()