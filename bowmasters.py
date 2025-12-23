import pygame
import sys
import random
import math

from abc import ABC, abstractmethod

# Initialize Pygame
pygame.init()

# Set up the display window
screen_width = 1700
screen_height = 900
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption("Physics window!!!")
gravity = 9.8
# Game loop control variable
running = True
FPS = 120

# Main game loop


rand_num_y = random.randint(-20, 20)
velocity = rand_num_y
clock = pygame.time.Clock()


BARRIER = 500



previous_points = []
frame = 0
initial_pos = ()
progress = 0
is_animating = False
y1 = 0
step = 1.28

score = 0
sticks = []
circles = []
index_launch = 0

substeps = 32
animation_num = 0
#pygame.mixer.music.load("background_music_physics_game.mp3")  # Works if MP3 is supported
#pygame.mixer.music.play(-1) 

start_time = pygame.time.get_ticks()
countdown_seconds = 30

# In your game loop:

trajectory = []
class ScoreAnimation:
    def __init__(self):

        
        
        self.animation1 = 150
        self.animation2 = 197.5
        self.animation3 = 150
        self.step = 0.009
        self.fade = False
        
        self.alpha = 255
        self.color_dict = {
            0: (128, 0, 128),
            5: (142, 28, 114),
            10: (156, 57, 99),
            15: (170, 85, 85),
            20: (185, 113, 71),
            25: (199, 142, 57),
            30: (213, 170, 42),
            35: (227, 198, 28),
            40: (241, 227, 14),
            45: (255, 255, 0)
        }
        self.surf = pygame.Surface((1700, 900), pygame.SRCALPHA)
        self.surf.fill((0, 0, 0, 0))   
        self.blitz_animation_x = 0
        self.blitz_animation_y = screen_height
    def round_to_five(self, num):
        return self.color_dict[min(int(num) - (int(num) % 5), 45)]


    def draw_vertical_gradient_rect(self, surface, rect, top_color, bottom_color):
        
        height = rect.height
        for y in range(height):
            ratio = y / height
            r = int(top_color[0] * (1 - ratio) + bottom_color[0] * ratio)
            g = int(top_color[1] * (1 - ratio) + bottom_color[1] * ratio)
            b = int(top_color[2] * (1 - ratio) + bottom_color[2] * ratio)
            pygame.draw.line(surface, (r, g,b, self.alpha), (rect.left, rect.top + y), (rect.right, rect.top + y))
    def draw_horizontal_gradient_rect(self, surface, rect, top_color, bottom_color):
        
        height = rect.height

        for y in range(rect.width):
            ratio = y / rect.width
            r = int(top_color[0] * (1 - ratio) + bottom_color[0] * ratio)
            g = int(top_color[1] * (1 - ratio) + bottom_color[1] * ratio)
            b = int(top_color[2] * (1 - ratio) + bottom_color[2] * ratio)
            pygame.draw.line(surface, (r, g,b, self.alpha), (rect.left+y, rect.top), (rect.left+y, rect.bottom))
    def fade_out(self, num):
        


        rect = pygame.Rect(screen_width/2 - 15, self.animation1, 30, 125)
        
        self.draw_vertical_gradient_rect(self.surf, rect, (0, 0, 0), self.round_to_five(num))
        rect2 = pygame.Rect(screen_width/2 - 125/2, self.animation2, 125, 30)
        self.draw_horizontal_gradient_rect(self.surf, rect2, (0, 0, 0), self.round_to_five(num))
        self.text_surface.set_alpha(self.alpha)
        
        if self.alpha == 0:
            self.alpha = 0
        else:
            
            self.alpha -= 2.5
            self.alpha = max(self.alpha, 0)
        screen.blit(self.text_surface, (screen_width/2 - 125/2 + 150, self.animation3/2))
        screen.blit(self.surf, (0, 0))
        self.surf.fill((0, 0, 0, 0))
    
    def draw_score_animation(self, num):
        font = pygame.font.SysFont("arial", 195)
        self.text_surface = font.render(num, True, self.round_to_five(num))
        #print(self.animation1)
        if not self.fade:
            if self.animation1 <= 50 and self.animation2 <= 97.5:
                self.animation1 = 50
                self.animation2 = 97.5   
                self.animation3 = 50
                self.fade = True
            rect = pygame.Rect(screen_width/2 - 15, self.animation1, 30, 125)
            self.draw_vertical_gradient_rect(self.surf, rect, (0, 0, 0), self.round_to_five(num))
            
            rect2 = pygame.Rect(screen_width/2 - 125/2, self.animation2, 125, 30)
            self.draw_horizontal_gradient_rect(self.surf, rect2, (0, 0, 0), self.round_to_five(num))
            self.animation3 -= 14
            
            self.animation1 -= 14
            self.animation2 -=  14
        else:
            self.fade_out(num)
        
    
        # Draw it on screen
        screen.blit(self.text_surface, (screen_width/2 - 125/2 + 150, self.animation3/2))
        screen.blit(self.surf, (0, 0))
        self.surf.fill((0, 0, 0, 0))
    def draw_blitz_mode(self):
        if self.blitz_animation_x >= 195 and self.blitz_animation_y >= screen_height - 195:
            self.blitz_animation_x = 195
            self.blitz_animation_y = screen_height - 195
        font = pygame.font.SysFont("arial", 195)
        self.text_surface = font.render("Blitz Mode", True, (255, 255, 0))
        screen.blit(self.text_surface, (self.blitz_animation_x, self.blitz_animation_x))
        self.blitz_animation_x += 15
        self.blitz_animation_y -= 15



class Quadtree:
    def __init__(self, boundary, capacity):
        self.boundary = boundary  # pygame.Rect
        self.capacity = capacity
        self.objects = []
        self.divided = False
        self.northwest = self.northeast = self.southwest = self.southeast = None


    def subdivide(self):
        x, y, w, h = self.boundary
        hw = w // 2
        hh = h // 2

        self.northwest = Quadtree(pygame.Rect(x, y, hw, hh), self.capacity)
        self.northeast = Quadtree(pygame.Rect(x + hw, y, w - hw, hh), self.capacity)
        self.southwest = Quadtree(pygame.Rect(x, y + hh, hw, h - hh), self.capacity)
        self.southeast = Quadtree(pygame.Rect(x + hw, y + hh, w - hw, h - hh), self.capacity)
        self.divided = True

    def insert(self, obj):
        obj_rect = self._get_obj_rect(obj)

        if not self.boundary.colliderect(obj_rect):
            return False

        if len(self.objects) < self.capacity and not self.divided:
            self.objects.append(obj)
            return True

        if not self.divided:
            self.subdivide()
            # reinsert existing
            old = self.objects
            self.objects = []
            for o in old:
                self.insert(o)

        # check if fits entirely inside ONE child
        for child in (self.northwest, self.northeast, self.southwest, self.southeast):
            if child.boundary.contains(obj_rect):
                return child.insert(obj)

        # if it doesn’t fit cleanly in one child, keep it here
        self.objects.append(obj)
        return True

    def query(self, range_rect, found=None):
        if found is None:
            found = []

        if not self.boundary.colliderect(range_rect):
            return found

        for obj in self.objects:
            if range_rect.colliderect(self._get_obj_rect(obj)):
                found.append(obj)

        if self.divided:
            self.northwest.query(range_rect, found)
            self.northeast.query(range_rect, found)
            self.southwest.query(range_rect, found)
            self.southeast.query(range_rect, found)

        return found


    def _get_obj_rect(self, obj):
        if isinstance(obj, Circle):
            return pygame.Rect(
                int(math.floor(obj.center_x - obj.radius)),
                int(math.floor(obj.center_y - obj.radius)),
                int(math.ceil(obj.radius * 2)),
                int(math.ceil(obj.radius * 2)),
            )
        elif isinstance(obj, Stick):
            # AABB that covers the line segment, but ensure non-zero size
            min_x = min(obj.Ax, obj.Bx)
            min_y = min(obj.Ay, obj.By)
            max_x = max(obj.Ax, obj.Bx)
            max_y = max(obj.Ay, obj.By)

            # ensure at least 1 pixel wide/tall so colliderect works
            width = max(5, int(math.ceil(max_x - min_x)))
            height = max(5, int(math.ceil(max_y - min_y)))

            left = int(math.floor(min_x))
            top = int(math.floor(min_y))

            return pygame.Rect(left, top, width, height)
        else:
            raise TypeError(f"Unsupported object type: {type(obj)}")

    def clear(self):
        self.objects = []
        self.divided = False
        self.northwest = self.northeast = self.southwest = self.southeast = None



class Shape(ABC):
    def __init__(self, vx=0, center_x=400, center_y=400, length=200, omega=0, angle = math.radians(90)):
        self.vy = 0
        self.vx = vx
        self.center_x = center_x
        self.angle = angle
        self.center_y = center_y
        self.length = length
        self.omega = omega
        self.gravity = 9.8
        self.mass = 10
        self.inertia = (1/12*self.mass*(self.length**2))
        self.linear_inertia = (1/12*(self.length**2))
        self.elasticity = 0.25
        self.allow_motion = False
        self.collision_frame_handled = False
        self.angle = self.angle % (2 * math.pi)
        self.draw_animation = False
    @abstractmethod
    def apply_motion(self):
        raise NotImplementedError
    @abstractmethod
    def apply_rotation(self):
        raise NotImplementedError
    @abstractmethod
    def x_boundaries(self):
        raise NotImplementedError
    @abstractmethod
    def y_boundaries(self):
        raise NotImplementedError
class Polygon(Shape):
    def __init__(self, vertices=4):
        super().__init__()
        self.vertices = vertices
        self.local_vertices = self.calculate_vertices()
        self.world_vertices = self.local_vertices
    def calculate_vertices(self):
        vertex_points = []
        R = self.length/(2*math.sin(math.pi/self.vertices))
        for i in range(self.vertices):
            x = self.center_x + R * math.cos((2*math.pi*i)/self.vertices + self.angle)
            y = self.center_y + R * math.sin((2*math.pi*i)/self.vertices + self.angle)
            vertex_points.append((x, y))
        return vertex_points



    def compute_line_segment_projection(self, segment_edge, segment_vertices: list, normal):
        


        
        
        projections = []

        for vertex in segment_vertices:
            x = vertex[0]
            y = vertex[1]
            p = x * normal[0] + y * normal[1]
            projections.append(p)
        min_projection = min(*projections)
        max_projection = max(*projections)
        projections.append((min_projection, max_projection))
        return projections
            


    def sat_algorithm_boundary(self, segment_edge, segment_vertices):
        edge_coordinates = []
        for i in range(len(self.world_vertices)):
            edge_coordinates.append((self.world_vertices[i], self.world_vertices[(i+1) % len(self.world_vertices)]))
        
        edges = []
        normals = []
        for edge in edge_coordinates:
            edge = (edge[0][0] - edge[1][0], edge[0][1] - edge[1][1])
            mag = math.sqrt(edge[0]**2 + edge[1]**2)

            
        
            nx = -edge[1] / mag
            ny = edge[0] / mag
            edges.append(edge)
            normals.append((nx, ny))

        seg_mag = math.sqrt(segment_edge[0]**2 + segment_edge[1]**2)

        
    
        seg_nx = -segment_edge[1] / seg_mag
        seg_ny = segment_edge[0] / seg_mag
        
        seg_normal = (seg_nx, seg_ny)
        normals.append(seg_normal)
        
        min_overlap = float('inf')
        mtv_axis = None

        
        for normal in normals:
            projections = []
            for vertex in self.world_vertices:
                x = vertex[0]
                y = vertex[1]
                p = x * normal[0] + y * normal[1]
                projections.append(p)
            poly_min = min(*projections)
            poly_max = max(*projections)
            other_projection = self.compute_line_segment_projection(segment_edge, segment_vertices, normal)
            seg_min, seg_max = other_projection
            

        

            if poly_max < seg_min or seg_max < poly_min:
                return False, 0  # separating axis found
            else:
                overlap = min(poly_max, seg_max) - max(poly_min, seg_min)
                if overlap < min_overlap:
                    min_overlap = overlap
                    mtv_axis = normal
                
        seg_center = (segment_vertices[0][0] + segment_vertices[1][0])/2, (segment_vertices[0][1] + segment_vertices[1][1])/2
        direction = (seg_center[0] - self.center_x, seg_center[1] - self.center_y)
        if direction[0] * mtv_axis[0] + direction[1] * mtv_axis[1] > 0:
            mtv_axis = (-mtv_axis[0], -mtv_axis[1])
        MTV = (mtv_axis[0] * min_overlap, mtv_axis[1] * min_overlap)
        return True, MTV

        



class Stick(Shape):
    def __init__(self, vx=3, center_x=400, center_y=400, length=200, omega=2, angle = math.radians(90)):
        super().__init__(vx, center_x, center_y, length, omega, angle)
        self.allow_motion = False
        self.collision_frame_handled = False
        self.angle = self.angle % (2 * math.pi)
        self.draw_animation = False

        dx = (self.length / 2) * math.cos(self.angle)
        dy = (self.length / 2) * math.sin(self.angle)
        self.collision_elasticity = 0.85
        self.Ax = self.center_x - dx
        self.Ay = self.center_y - dy
        self.Bx = self.center_x + dx
        self.By = self.center_y + dy
        self.collided = False
        self.remove = False
    def apply_motion(self, time):
        """Changes actual stick position over time using velocities that update the center points.
        That center then computes the endpoints (needed for pygame to render the image) using the length 
        of the stick and trig to find the rotation of the stick."""
        self.angle = self.angle % (2 * math.pi)
        if self.length < 200:
            pass
            #print(self.vx)

        dx = (self.length / 2 ) * math.cos(self.angle)
        dy = (self.length / 2) * math.sin(self.angle)

        self.Ax = self.center_x - dx
        self.Ay = self.center_y - dy
        self.Bx = self.center_x + dx
        self.By = self.center_y + dy
        
        self.vy += self.gravity * time
        #print(self.vy)
        self.center_y += self.vy * time
        #print(self.center_y)
        self.center_x += self.vx * time
        self.angle -= self.omega*time/12
    def clamp_motion(self):
        """Friction like motion so things dont go on forever"""
        
        self.vx *= .9999995
        
        if abs(self.vx) < 0.01:
            self.vx = 0

       # print("vy", self.vy)
       # print("vx", self.vx)
       # print("Omega", self.omega)
        velocity_magnitude = math.sqrt(self.vx**2 + self.vy**2)
        #When velocity is small, increase omega damping
        if velocity_magnitude < 0.1:
            self.omega /= 1.1 # stronger damping when slow
        else:
            self.omega *= .993

    def apply_rotation(self, lever_arm_r, Jx, Jy):
        
       
        T_impulse = ((lever_arm_r[0])*Jy-(lever_arm_r[1])*Jx)
        # print(f"T_impulse: {T_impulse}")
        omega_change = T_impulse/self.inertia
        # print(f"OMEGA: {omega_change}")
        self.omega -= omega_change
        
        

    def y_boundaries(self):
        """Whether the stick hits the ground, and to apply a change in velocity & rotation if it does."""
        # Just delete the stick if it is mostly stationary
        if self.Ay >= screen_height - 5 and self.By >= screen_height - 5:

            self.Ay = screen_height-5 
            self.By = screen_height-5 
            self.vx = 0
            self.vy = 0
            self.omega = 0
            self.remove = True
        elif self.Ay >= screen_height:
                
                
                lever_arm_r, Jx, Jy, vx, vxy, vy, dx, dy = self.static_collisions_helper(self.Ay, self.Ax, vx=self.vx, vy=self.vy)
                
                
                self.vy = vy
                self.vx = vxy
                self.center_y -= dy
                
                
                self.apply_rotation(lever_arm_r, Jx=0, Jy=Jy)
                


                
        elif self.By >= screen_height:
                
                
                lever_arm_r, Jx, Jy, vx, vxy, vy, dx, dy = self.static_collisions_helper(self.By, self.Bx, vx=self.vx, vy=self.vy)
                
                
                
                self.vy = vy
                self.vx = vxy
                self.center_y -= dy
                
                self.apply_rotation(lever_arm_r, Jx=0, Jy=Jy)
               


    def static_collisions_helper(self, endpoint1, endpoint2, vx, vy):
        
        
        
        dx = endpoint2 - screen_width
        
        dy =  endpoint1 - screen_height
        previous_velocity_y = vy
        #print(previous_velocity_y)
        previous_velocity_x = vx
        vxy = vx * 0.9
        if vy > 0:
            
            vy *= -self.elasticity
            
            
            
        if vx > 0:
            
            vx *= -self.elasticity
            
        collision_point_P = (endpoint2, endpoint1)
        lever_arm_r = (collision_point_P[0] - self.center_x, collision_point_P[1] - self.center_y)
        Jx = (vx - previous_velocity_x) * self.mass
        Jy = (vy - previous_velocity_y) * self.mass
        
        
        
        return lever_arm_r, Jx, Jy, vx, vxy, vy, dx, dy

    def x_boundaries(self):
        """Whether the stick hits the right wall, and to apply a change in velocity & rotation if it does."""
        if self.Ax >= screen_width:
                

                lever_arm_r, Jx, Jy, vx, vxy, vy, dx, dy = self.static_collisions_helper(self.Ay, self.Ax, vx=self.vx, vy=self.vy)
                self.vx = vx
                
                self.center_x -= dx
                self.apply_rotation(lever_arm_r, Jx=Jx, Jy=0)

                
        elif self.Bx >= screen_width:

                           
                lever_arm_r, Jx, Jy, vx, vxy, vy, dx, dy = self.static_collisions_helper(self.By, self.Bx, vx=self.vx, vy=self.vy)
                self.vx = vx
                
                self.center_x -= dx
                self.apply_rotation(lever_arm_r, Jx, 0)


    
    def _break_apart(self, point_of_contact, Jx, Jy):
        global score, animation_num
        vector_AB = (stick.Bx - stick.Ax, stick.By - stick.Ay)
        vector_AP = (point_of_contact[0] - stick.Ax, point_of_contact[1] - stick.Ay)
        dot_product = (vector_AB[0] * vector_AP[0] + vector_AB[1] * vector_AP[1])
        AB_squared = (vector_AB[0] * vector_AB[0] + vector_AB[1] * vector_AB[1])
        if AB_squared == 0:
            return
        t = dot_product/AB_squared
        if t < 0 or t > 1:
            return
        Px = stick.Ax + t * vector_AB[0]
        Py = stick.Ay + t * vector_AB[1]
        vector_top = (Px - stick.Ax, Py - stick.Ay)
        distance_top = math.sqrt(vector_top[0]**2 + vector_top[1]**2)

        
        vector_bottom = (Px - stick.Bx, Py - stick.By)
        distance_bottom = math.sqrt(vector_bottom[0]**2 + vector_bottom[1]**2)
        self.length = math.sqrt((stick.Bx - stick.Ax)**2 + (stick.By - stick.Ay)**2)
        assert abs((distance_top + distance_bottom) - self.length) < 1e-6
        
        

        #print(f"BREAKAGE: {self.vx}")

        self.vy -= Jy/self.mass
      

        
    
        
        self.vx -= Jx/self.mass
        stick1 = Stick(center_x=point_of_contact[0], center_y=point_of_contact[1] + 10, length=max(6, distance_top), angle=self.angle, vx=self.vx, omega = self.omega)
        
        stick2 = Stick(center_x=point_of_contact[0], center_y=point_of_contact[1]+distance_bottom + 10, length=max(6, distance_bottom), angle=self.angle, vx=self.vx, omega = -self.omega*3 )
        stick1.allow_motion = True
        stick2.allow_motion = True
        stick1.collided = True
        stick2.collided = True
        
        sticks.append(stick1)
        sticks.append(stick2)
        #print(self.allow_motion)
        if not (self.allow_motion) :
            score += 5
            animation_num +=5
        stick1.draw_animation = True
        stick2.draw_animation = True
        self.remove = True    
        
        
        

    def _segments_intersect(self, p1, p2, q1, q2):
        def ccw(a, b, c):
            return (c[1]-a[1])*(b[0]-a[0]) > (b[1]-a[1])*(c[0]-a[0])
        return (ccw(p1, q1, q2) != ccw(p2, q1, q2)) and (ccw(p1, p2, q1) != ccw(p1, p2, q2))


    def line_intersects_segment(self, start, end):
        stick_start = (self.Ax, self.Ay)
        stick_end = (self.Bx, self.By)
        return self._segments_intersect(start, end, stick_start, stick_end)
    
    def draw_stick(self, allow_motion):
        if not allow_motion:
            pygame.draw.line(screen, (255, 255, 255), (self.Ax, self.Ay), (self.Bx, self.By), 5)
        else:
            pygame.draw.line(screen, (255, 0, 0), (self.Ax, self.Ay), (self.Bx, self.By), 5)

class Circle(Shape):
    def __init__(self, initial_x=150, initial_y = 300):
        super().__init__(0, initial_x, initial_y)
        self.elasticity = 0.95
       
        
        self.mass = 1
        self.clock = pygame.time.Clock()
        
        self.allow_motion = True

        self.name = ""
        
        self.radius = 40

    def update_velocity_drag(self, initial_pos, mouse_release_pos):
        dx = (initial_pos[0] - mouse_release_pos[0])

        dy = (initial_pos[1] - mouse_release_pos[1])
        
        #print(initial_pos)
        
        flight_time = 1.5
        self.vx = dx/flight_time
        self.vy = dy/flight_time

        

    def draw_trajectory(self, initial_pos, mouse_pos_x, mouse_pos_y):
        global trajectory
        trajectory.append([])

        #print(velocity, velocity_x)
        projected_velocity_x = (initial_pos[0] - mouse_pos_x)/1.5
        projected_velocity_y = (initial_pos[1] - mouse_pos_y)/1.5
        flight_time = 0.5
        for i in range(1, 300):

            t = i/flight_time/FPS
            x = mouse_pos_x + projected_velocity_x * t
            y = mouse_pos_y + projected_velocity_y * t + 0.5 * 9.8 * (t)**2
            trajectory[-1].append((x, y))
        for i, point in enumerate(trajectory[-1]):
            pygame.draw.circle(screen, (255, 255, 255), point, 5)
        trajectory = trajectory[-1]

    def remove_if_outofbounds(self):
        if self in circles and self.name != "main_circle":
            
            if self.center_x - 20 > screen_width:
                circles.remove(self)
            elif self.center_y - 20 > screen_height or self.center_y + 20 < 0:
                circles.remove(self)
    def apply_rotation(self):
        return 0

    def apply_motion(self, dt):
        if self.allow_motion:
           
            self.vx *= 0.999
            # seconds since last frame
            self.vy += self.gravity * dt
            
            #print(self.velocity)
            self.center_y += self.vy * dt
            self.center_x += self.vx * dt
    def y_boundaries(self):
        if self.center_y >= screen_height - 40:
            self.center_y = screen_height - 40
            self.vy *= -self.elasticity
    def x_boundaries(self):
        if self.center_x <= 40:
            self.center_x = 40
            self.vx *= -self.elasticity      
    def draw_circle(self):
        pygame.draw.circle(screen, (128, 0, 128), (self.center_x, self.center_y), 40, 3)
class CollisionDispatcher:
    def __init__(self):
        self._handlers = {}

    def register(self, typeA, typeB, func):
        self._handlers[(typeA, typeB)] = func
        self._handlers[(typeB, typeA)] = lambda b, a: func(a, b)

    def detect_and_resolve(self, shapeA, shapeB):
        func = self._handlers.get((type(shapeA), type(shapeB)))
        if func:
            return func(shapeA, shapeB)
        return None



class CollisionDetection:
    

    def _compute_rotational_change(self, stick: Stick, lever_arm_r, Jx, Jy):
            """Calculates the amount of omega change needed after the collision, determines spinning.
            The lever arm is the segment from the axis of rotation (center in this case) that determines
            how much torque should be applied. Jx and Jy are translational impulses."""
           
            T_impulse = ((lever_arm_r[0])*Jy-(lever_arm_r[1])*Jx)
            # print(f"T_impulse: {T_impulse}")
            omega_change = T_impulse/stick.inertia
            # print(f"OMEGA: {omega_change}")
            stick.omega += omega_change
           
    def _calculate_distance(self, circle: Circle, stick: Stick):
        """Calculates distance between the circle and the stick."""

        vector_AB = (stick.Bx - stick.Ax, stick.By - stick.Ay)
        vector_AC = (circle.center_x - stick.Ax, circle.center_y - stick.Ay)
        dot_product = (vector_AB[0] * vector_AC[0] + vector_AB[1] * vector_AC[1])
        AB_squared = (vector_AB[0] * vector_AB[0] + vector_AB[1] * vector_AB[1])
        if AB_squared == 0:
            return
        t = dot_product/AB_squared
        t = max(0, min(1, t))
        Px = stick.Ax + t * vector_AB[0]
        Py = stick.Ay + t * vector_AB[1]
        closest_point = (Px, Py)
        dx = closest_point[0] - circle.center_x
        dy = closest_point[1] - circle.center_y
        distance = math.sqrt(dx**2 + dy **2)
        if distance == 0:
            mag = math.sqrt(vector_AB[0]**2 + vector_AB[1]**2)
            nx = vector_AB[0]/mag
            ny = -vector_AB[1]/mag
            
        else:
            nx = dx / distance
            ny = dy / distance
        
        penetration = max(0, circle.radius + 8 - distance) 
        contact_x = circle.center_x + nx * (penetration) 
        contact_y = circle.center_y + ny * (penetration)
        
        return distance, (contact_x, contact_y)


    def _impulse_resolution(self, penetration, closest_point, circle: Circle, stick: Stick, dt):
        """Collision logic based on realistic physics principles. Penetration is how much the circle and the stick are pushing in
        to each other, closest point is the point of contact between the circle and stick, Cvx and Cvy are circle velocities and Cx, Cy are
        Circle center points.""" 
            
        
        
        lever_arm_r = (closest_point[0] - stick.center_x, closest_point[1] - stick.center_y)
        #   print(f"lever arm r: {lever_arm_r}")
        
        
        r = lever_arm_r
        #Computes rotational velocity of the stick

        v_rot_x = stick.omega * r[0]
        v_rot_y = -stick.omega * r[1]

    
        #computes relative velocity, adding rot and translational velocity together
        v_contact_x = stick.vx + v_rot_x
        v_contact_y = stick.vy + v_rot_y
        
        
        #computes the velocity at the point of contact between the circle's vel and the sticks relative vel
        stick_contact_point_x = circle.vx - v_contact_x
        stick_contact_point_y = circle.vy - v_contact_y
        
        dx = (circle.center_x - closest_point[0])
        
        dy = circle.center_y  - closest_point[1]

        
        mag = math.sqrt(dx*dx + dy*dy)
        
        # Computes the collision normal, i.e. the direction the impulse is facing.
        collision_normal_x = dx / mag
        collision_normal_y = dy / mag
        


        # normalizes direction
        v_rel_dot_n =  stick_contact_point_x * collision_normal_x + stick_contact_point_y * collision_normal_y
        
        v0, v1 = 0.1, 1.0
        t = max(0.0, min(1.0, (-v_rel_dot_n - v0) / (v1 - v0)))
        stick.collision_elasticity = stick.collision_elasticity * t
        
        
        impulse_scalar = (-(1 + stick.collision_elasticity) * min(v_rel_dot_n, 0))/(1/circle.mass + 1/stick.mass + ((r[0] * collision_normal_y - r[1] * collision_normal_x)**2)/stick.inertia)

        bias = 0.3 * max(penetration - 0.01, 0)/dt

        
        Jx = ((impulse_scalar+bias) * collision_normal_x)
        
        Jy = ((impulse_scalar+bias) * collision_normal_y)
        self._compute_rotational_change(stick, lever_arm_r, Jx, Jy)
        self._should_break(stick, circle, closest_point, v_rel_dot_n, Jx, Jy)
            
        return closest_point, lever_arm_r, v_rel_dot_n, Jx, Jy
    def _should_break(self,stick: Stick, circle: Circle,  closest_point, v_rel_dot_n, Jx, Jy):
        """Center hits are more likely to break"""
        
        base_threshold = 125

        v0, v1 = 0.1, 5.0
       
        vector = (closest_point[0] - stick.center_x, closest_point[1] - stick.center_y)
        distance = math.sqrt(vector[0]**2 + vector[1]**2)
        length_factor = 100 /( stick.length /circle.mass)  # longer sticks → smaller threshold
       
        
        threshold = max(0.1, (distance - v0) / (v1 - v0)) * base_threshold * length_factor
        if abs(v_rel_dot_n) > threshold:
            stick._break_apart(closest_point, Jx, Jy)
                
  



    def collision_detection(self, circle: Circle, stick: Stick, dt):
        """Main public facing function for collision detection for the game. Mainly used for collisions between a circle and a stick. 
        Cx, Cy is the center point of the circle and Cvy and Cvx are the velocities of the circle. Implements translational velocity as well as rotational
        velocity impulses over dt.
        """
        if circle.center_x < 300:
            return
        try:
            distance, closest_point = self._calculate_distance(circle, stick)
        except TypeError:
            return
       
        if distance <= circle.radius + 8: # circle radius + stick width

            penetration =  circle.radius + 8 - distance 
            result = self._impulse_resolution(penetration, closest_point, circle, stick, dt)
            if result:
                closest_point, lever_arm_r, v_rel_dot_n, Jx, Jy = result
            else:
                return

          
            stick.vy -= Jy/stick.mass
         

           
        
           
            stick.vx -= Jx/stick.mass
          
            circle.vy += Jy/circle.mass
        

           
        
           
            circle.vx += Jx/circle.mass
            stick.vy = max(min(stick.vy, 200), -200)
            stick.vx = max(min(stick.vx, 200), -200)
          

            
            stick.collided = True
            stick.allow_motion = True            

                

                
            return True
        return False




def get_intersection_point(start1, end1, start2, end2):
    def subtract(a, b):
        return [a[0] - b[0], a[1] - b[1]]

    def add(a, b):
        return [a[0] + b[0], a[1] + b[1]]

    def multiply(scalar, vector):
        return [scalar * vector[0], scalar * vector[1]]

    def cross(a, b):
        return a[0] * b[1] - a[1] * b[0]

    p = list(start1)
    r = subtract(end1, start1)
    q = list(start2)
    s = subtract(end2, start2)

    cross_r_s = cross(r, s)
    if abs(cross_r_s) < 1e-10:  # Lines are parallel
        return None

    t = cross(subtract(q, p), s) / cross_r_s
    u = cross(subtract(q, p), r) / cross_r_s

    if 0 <= t <= 1 and 0 <= u <= 1:
        intersection = add(p, multiply(t, r))
        return tuple(intersection)
    else:
        return None



def draw_barrier():
    global y1

    
    

        
    
    bright_red = pygame.draw.line(screen, (255, 0, 0), (BARRIER, 0), (BARRIER, y1), 3)
    dark_red = pygame.draw.line(screen, (139, 0, 0), (BARRIER+10, 0), (BARRIER+10, y1), 17)
    bright_red2 = pygame.draw.line(screen, (255, 0, 0), (BARRIER+20, 0), (BARRIER+20, y1), 3)
    
    y1 = bright_red.top + (bright_red.bottom - bright_red.top) * step
    if y1 >= 900:
        y1 = 900
def do_every(interval_ms, *, num_times=0):
    """Returns True if the interval has passed since the last True."""
    now = pygame.time.get_ticks()
    do_every.num_times = num_times
    if not hasattr(do_every, "last_time"):
        do_every.last_time = 0  # first call initialization
    
    if do_every.num_times > num_times and do_every.num_times != 0:
        return False
    if now - do_every.last_time >= interval_ms:
        do_every.last_time = now
        do_every.num_times = 0
        return True
    else:
        num_times += 1
        return False
    
    

def raycaster(circle: Circle):
    global trajectory, score, animation_num
    if abs(main_circle.vx) > 450 or abs(main_circle.vy) > 450:
        for i, point in enumerate(trajectory):
            if not trajectory:
                break
            start = trajectory[i]
            if i < 290:
                end = trajectory[i + 1]
            
            
                for stick in sticks:
                    stick: Stick
                    if stick.line_intersects_segment(start, end):
                        # Collision detected
                        # Option 1: Stop the loop and resolve here
                    #  print(f"velocity_x: {velocity_x}")
                        collision_point = get_intersection_point(start, end, (stick.Ax, stick.Ay), (stick.Bx, stick.By))

                        # Move the circle to this point
                        collision_point = (int(collision_point[0]) - circle.radius, int(collision_point[1]))
                        
                        
                #     print(stick.center_y)

                        main_circle.center_x, main_circle.center_y = collision_point
                        
                        #print(main_circle.velocity, main_circle.velocity_x)
                        #print(f"collision point: {collision_point}")
                        
                       
                        
                        
                        stick.allow_motion = True
                        
                        #score, animation_num = create_new_stick(score, animation_num, animation, stick)

                                        
                        
                        create_new_stick(score, animation_num, animation, stick)
                        
                        # Break out of the loop to avoid multiple collisions
                        trajectory = []  
                        return
            
            
        trajectory = []           
    


def show_fps():
    pass
def show_score(font):
    label_text = f"Score: {score}"

    label_color = (255,255,255)
    label_surface = font.render(label_text, True, label_color)

    
    # Label position
    label_pos = (50, 50)
    screen.blit(label_surface, label_pos) 
    current_fps = f"FPS: {(clock.get_fps()):.2f}"
    fps_surface = font.render(current_fps, True, label_color)
    fps_pos = (600, 50)
    screen.blit(fps_surface, fps_pos)

animation = ScoreAnimation()
stick = Stick()
main_circle = Circle()
collision_detection = CollisionDetection()

main_circle.name = "main_circle"

circles.append(main_circle)
for i in range(6):
    circles.append(Circle())

sticks.append(stick)
def create_new_stick(score, animation_num, animation: ScoreAnimation, stick: Stick):
    sticks_not_collided = [stick for stick in sticks if not stick.collided]
    if len(sticks_not_collided) <= 0:
        score +=1
        animation_num +=1
        animation.alpha = 255
        stick.draw_animation = True
        new_stick = Stick(vx = 3, center_x = random.randint(500, 1500), center_y = random.randint(200, 700), length = 250, omega = 2, angle = math.radians(random.randint(0, 180)))
        
        
        sticks.append(new_stick)
        return score, animation_num
    return score, animation_num

class CircleGame:
    def __init__(self, screen, clock, FPS, circles, sticks, main_circle, animation, 
                 screen_height, BARRIER, collision_detection, create_new_stick,
                 show_score, raycaster, do_every, countdown_seconds):
        
        self.screen = screen
        self.clock = clock
        self.FPS = FPS
        self.circles: list[Circle] = circles
        self.sticks: list[Stick]= sticks
        self.main_circle: Circle = main_circle
        self.animation = animation
        self.screen_height = screen_height
        self.BARRIER = BARRIER
        self.collision_detection = collision_detection
        self.create_new_stick = create_new_stick
        self.show_score = show_score
        self.raycaster = raycaster
        self.do_every = do_every
        self.countdown_seconds = countdown_seconds
        self.dt = 12/120
        self.running = True
        self.initial_pos = pygame.mouse.get_pos()
        self.score = 0
        self.animation_num = 0
        self.index_launch = 0
        self.gravity = 9.8
        self.start_time = pygame.time.get_ticks()

    def handle_events(self, events):
        self.quadtree = Quadtree(pygame.Rect(0, 0, 1700, 900), 4)
        for event in events:
            if event.type == pygame.QUIT:
                self.running = False

            elif event.type == pygame.MOUSEBUTTONDOWN:
                for circle in self.circles:
                    circle.allow_motion = False
                self.initial_pos = pygame.mouse.get_pos()

            elif event.type == pygame.MOUSEBUTTONUP:
                for circle in self.circles:
                    circle.allow_motion = True
                    mouse_release_pos = pygame.mouse.get_pos()
                    circle.update_velocity_drag(self.initial_pos, mouse_release_pos)

                for circle in self.circles.copy():
                    if circle.name != "main_circle":
                        self.circles.remove(circle)

                self.index_launch = 0
                self.gravity = 9.8
                self.raycaster(main_circle)

    def update_circles_and_sticks(self):
        for circle in self.circles:
            self.quadtree.insert(circle)
        for stick in self.sticks:
            self.quadtree.insert(stick)

        for c1 in self.circles:
            area = pygame.Rect(c1.center_x - c1.radius, c1.center_y - c1.radius, c1.radius*2, c1.radius*2)
            nearby = self.quadtree.query(area)
            nearby_sticks = sorted([s for s in nearby if not isinstance(s, Circle)], key=id)

            for stick in nearby_sticks:
                if c1 == stick or isinstance(stick, Circle):
                    continue

                collision = self.collision_detection.collision_detection(c1, stick, self.dt)


                if stick.collided and stick.length >= 25:
                    self.score, self.animation_num = self.create_new_stick(self.score, self.animation_num, self.animation, stick)

        for circle in self.circles.copy():
            buttons = pygame.mouse.get_pressed()
            if not buttons[0]:
                circle.y_boundaries()
                circle.x_boundaries()
            circle.apply_motion(self.dt)
            circle.draw_circle()

        for circle in self.circles.copy():
            if len(self.circles) > 1:
                circle.remove_if_outofbounds()

        for stick in self.sticks.copy():
            stick.draw_stick(stick.allow_motion)
            stick.clamp_motion()
            if stick.allow_motion:
                stick.apply_motion(self.dt)
                stick.y_boundaries()
                stick.x_boundaries()
                if stick.remove:
                    try:
                        self.sticks.remove(stick)
                    except ValueError:
                        pass

    def draw_timer(self):
        font = pygame.font.SysFont("arial", 32)
        elapsed_ms = pygame.time.get_ticks() - self.start_time
        remaining_time = max(0, self.countdown_seconds - elapsed_ms // 1000)

        if remaining_time <= 0:
            self.start_time = pygame.time.get_ticks()
            elapsed_ms = pygame.time.get_ticks() - self.start_time
            while elapsed_ms < 2000:
                elapsed_ms = pygame.time.get_ticks() - self.start_time
                self.screen.fill((0, 0, 0))
                timer_surface = font.render(f"Time's up! Your score is: {self.score}", True, (0, 255, 0))
                self.screen.blit(timer_surface, (200, 50))
                pygame.display.flip()
                self.clock.tick(30)
            self.score = 0
            self.start_time = pygame.time.get_ticks()

        elif 5 < remaining_time <= 15:
            timer_surface = font.render(f"Seconds Remaining: {remaining_time}", True, (255, 255, 0))
            self.screen.blit(timer_surface, (225, 50))
            self.show_score(font)

        elif remaining_time <= 5:
            timer_surface = font.render(f"Seconds Remaining: {remaining_time}", True, (255, 0, 0))
            self.screen.blit(timer_surface, (225, 50))
            self.show_score(font)

        else:
            timer_surface = font.render(f"Seconds Remaining: {remaining_time}", True, (255, 255, 255))
            self.screen.blit(timer_surface, (225, 50))
            self.show_score(font)

    def run(self, events):
    
        self.handle_events(events)

        self.screen.fill((0, 0, 0))
        buttons = pygame.mouse.get_pressed()

        if buttons[0]:
            self.gravity = 0
            pos = pygame.mouse.get_pos()

            for circle in self.circles:
                if not circle.allow_motion:
                    circle.center_x, circle.center_y = pos

            all_circles_no_motion = [c for c in self.circles if not c.allow_motion]
            if all_circles_no_motion:
                self.main_circle.draw_trajectory(self.initial_pos, self.main_circle.center_x, self.main_circle.center_y)

            if self.circles and self.animation_num > 25:
                if self.do_every(125):
                    new_circle = Circle(pos[0], pos[1])
                    new_circle.allow_motion = False
                    self.circles.append(new_circle)
                    if self.index_launch > len(self.circles) - 1:
                        self.index_launch = 0

                    circle_to_release = self.circles[self.index_launch]
                    if circle_to_release.name == "main_circle":
                        circle_to_release = new_circle

                    circle_to_release.allow_motion = True
                    circle_to_release.gravity = 9.8
                    circle_to_release.update_velocity_drag(self.initial_pos, pos)
                    self.index_launch += 1
            else:
                self.animation.blitz_animation_x = 0
                self.animation.blitz_animation_y = self.screen_height

        if len(self.circles) > 0 and self.animation_num > 25:
            self.animation.draw_blitz_mode()

        self.clock.tick(self.FPS)
        self.dt = 12 / self.clock.get_fps() if self.clock.get_fps() != 0 else 2 / 120

        for stick in self.sticks:
            if stick.draw_animation:
                if self.animation_num != 0:
                    self.animation.draw_score_animation(str(self.animation_num))
                    if self.animation.alpha == 0:
                        stick.draw_animation = False
                        self.animation = ScoreAnimation()
                        self.animation_num = 0
                break

        self.update_circles_and_sticks()
        self.draw_timer()
        

class Play:
    def __init__(self):
        self.game = CircleGame(screen, clock, FPS, circles, sticks, main_circle, animation, 
        screen_height, BARRIER, collision_detection, create_new_stick,
        show_score, raycaster, do_every, countdown_seconds)
    def play(self, events):

        self.game.run(events)
        
