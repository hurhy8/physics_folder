import pygame
import pygame.gfxdraw
import physics
import random
import math
from animation import ScoreAnimation
from itertools import chain
collision = physics.PolygonCollisions()
pendulum_solver = physics.PendulumSolver()
man = physics.PolygonManager(collision, pendulum_solver)
pygame.init()
SCREEN_HEIGHT = 900
SCREEN_WIDTH = 1700
class Timer:
    def __init__(self):
        self.last_time = 0
        self.trigger_count = 0

        
        self.start_time = pygame.time.get_ticks()
    def do_every(self, interval_ms, *, max_times=None):
        """Return True if 'interval_ms' milliseconds have passed since last True."""
        now = pygame.time.get_ticks()

        # Check if enough time has elapsed
        if now - self.last_time >= interval_ms:
            self.last_time = now
            self.trigger_count += 1

            # If we have a limit on how many times to trigger
            if max_times is not None and self.trigger_count > max_times:
                return False

            return True

        return False
    def count_down(self, seconds):
        now = pygame.time.get_ticks()
        elapsed = (now - self.start_time) // 1000
        remaining = seconds - elapsed

        # Only return something when a new full second passes
        if remaining <= 0:
            return 0      
        self.last_second = elapsed
        return remaining  # e.g., 4, 3, 2, 1...

        # When finished


        
    def reset(self):
        self.start_time = pygame.time.get_ticks()
        self.last_time = self.start_time
        self.trigger_count = 0


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
def show_score(score, screen, font):
    label_text = f"Score: {score}"
    label_surface = font.render(label_text, True, (255, 255, 255))

    
    # Label position
    label_pos = (50, 50)
    screen.blit(label_surface, label_pos) 
def create_star(mx, my):
    children2 = []
    L = 260
    R = L * 0.3

    for i in range(5):
        ang = math.radians(i * 360/5)

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

class GameLogic:
    def __init__(self,screen, font, shape: physics.Shape, anim_num = 1):

        self.anim_num = anim_num
        
        self.polygon_main = shape
        
        self.sticks = []
        self.polygon_main.name = "main_polygon"
        #polygon2 = physics.Polygon(center_x=900, vx=1500)
        man.add_polygon(self.polygon_main)

        #man.add_polygon(polygon2)
        self.polygons = [self.polygon_main]
        first_stick = physics.Rectangle(width=25, length=200, center_x=600, center_y=500)
        first_stick.initialize_self_vector()
        first_stick.allow_motion= False
        self.sticks.append(first_stick)
        man.add_polygon(first_stick)
        self.BARRIER = 500
        self.initial_pos = ()
        self.y1 = 0
        self.step = 1.28
        self.score = 0
        self.index_launch = 0
        self.animation_num = 0
        self.running = True
        #pygame.mixer.music.load("background_music_physics_game.mp3")  # Works if MP3 is supported
        #pygame.mixer.music.play(-1) 
        self.start_time = pygame.time.get_ticks()
        self.countdown_seconds = 30
        self.animation = ScoreAnimation()
        self.timer = Timer()
        self.screen = screen
        self.font = font
        self.velocity_threshold = 0
    def create_new_stick(self, stick: physics.Rectangle, score_increase=1):
        sticks_not_collided = [stick for stick in self.sticks if not stick.collided]
        if len(sticks_not_collided) <= 0:
            self.score +=score_increase
            self.animation_num +=score_increase
            self.animation.alpha = 255
            stick.draw_animation = True
            new_stick = physics.Rectangle(vertices=4, vx = 0, center_x = random.randint(500, 1500), center_y = random.randint(200, 700), length = 200, width=25, omega = 0)
            
            new_stick.angle = (random.randint(0, 180))
            max_att = 10
            att = 0
            collision_free = False
            while not collision_free and att <= max_att:
                collision_free = True
                
                for polygon in chain(self.sticks, self.polygons):
                   
                    polygon: physics.Polygon
                    if new_stick.AABB_overlap(new_stick.aabb, polygon.aabb):
                        collision_free = False
                        new_stick = physics.Rectangle(vertices=4, vx = 0, center_x = random.randint(500, 1500), center_y = random.randint(200, 700), length = 200, width=25, omega = 0)
                        
                        new_stick.angle = (random.randint(0, 180))
                att += 1

               
            new_stick.allow_motion = False

            new_stick.initialize_self_vector()
            self.sticks.append(new_stick)
            man.add_polygon(new_stick)

    def handle_event(self, event: pygame.event.Event):
        if event.type == pygame.QUIT:  # Check if the user clicked the close button
            self.running = False
        if event.type == pygame.MOUSEBUTTONDOWN:
            pos = pygame.mouse.get_pos()
            #new_polygon = physics.Polygon(self.vertices, length=self.length, mass=self.mass, center_x=pos[0], center_y=pos[1], omega=0, vx=0)
            #new_polygon.initialize_self_vector()
            #man.add_polygon(new_polygon)
            #self.polygons.append(new_polygon)
            #new_polygon.angle = (math.pi * (self.vertices - 2))/(2*self.vertices)
            for polygon in self.polygons:
                polygon.allow_motion = False
            
            self.initial_pos = pygame.mouse.get_pos()
            

        if event.type == pygame.MOUSEBUTTONUP:
            for polygon in self.polygons:
                polygon.allow_motion = True
                mouse_release_pos = pygame.mouse.get_pos()
                update_velocity_drag(polygon, self.initial_pos, mouse_release_pos)
            for polygon in self.polygons.copy():
                if polygon.name != "main_polygon":
                    self.polygons.remove(polygon)
                    man.remove_polygon(polygon)
            self.index_launch = 0
            
            self.polygon_main.gravity = 9.8
            self.y1 = 0
    def draw_barrier(self):
        

        
        

            
        
        bright_red = pygame.draw.line(self.screen, (255, 0, 0), (self.BARRIER, 0), (self.BARRIER, self.y1), 3)
        pygame.draw.line(self.screen, (139, 0, 0), (self.BARRIER+10, 0), (self.BARRIER+10, self.y1), 17)
        pygame.draw.line(self.screen, (255, 0, 0), (self.BARRIER+20, 0), (self.BARRIER+20, self.y1), 3)
        
        self.y1 = bright_red.top + (bright_red.bottom - bright_red.top) * self.step
        if self.y1 >= 900:
            self.y1 = 900
    def _launch_logic(self):
 # Update the index and spawn a new polygon
        pos = pygame.mouse.get_pos()        
        new_polygon = physics.Polygon(self.vertices, length=self.length, mass=self.mass, center_x=pos[0], center_y=pos[1])
        new_polygon.initialize_self_vector()
        self.polygons.append(new_polygon)
        new_polygon.allow_motion = False

        # Select and prepare the polygon to release
        try:
            polygon_to_release: physics.Polygon = self.polygons[self.index_launch]
        except IndexError:
            polygon_to_release = new_polygon
        
        if polygon_to_release.name != "polygon_main":
            
            polygon_to_release.gravity = 9.8

            # Update velocity and position

            update_velocity_drag(polygon_to_release, self.initial_pos, pos)
            
            polygon_to_release.allow_motion = True
            

            man.add_polygon(polygon_to_release)
            self.index_launch = (self.index_launch + 1) % len(self.polygons)
    def blitz_logic(self):
        self.velocity_threshold = 600
        pos = pygame.mouse.get_pos()
        for polygon in self.polygons:
            if not polygon.allow_motion:
                polygon.center_x = pos[0]
                polygon.center_y = pos[1]
        if self.animation_num > self.anim_num:
            if self.polygon_main in self.polygons:
                self.polygons.remove(self.polygon_main)
                man.remove_polygon(self.polygon_main)

            

            if self.timer.do_every(350):
                self._launch_logic()

               
        else:
            self.velocity_threshold = 150
            for polygon in self.polygons.copy():
                if polygon.name != "main_polygon":
                    self.polygons.remove(polygon)
                    man.remove_polygon(polygon)
            if self.polygon_main not in self.polygons:
                self.polygons.append(self.polygon_main)
                man.add_polygon(self.polygon_main)
            self.animation.blitz_animation_x = 2
            
            self.polygon_main.center_x = pos[0]
        
            self.polygon_main.center_y = pos[1]
            
            self.polygon_main.sleep = False
            for stick in self.sticks:
                stick.sleep = False
        if len(self.polygons) > 0 and self.animation_num > self.anim_num:
            
            self.animation.draw_blitz_mode()   
    def handle_animation_state(self):
        stick_anim = [stick for stick in self.sticks if stick.draw_animation]
        if  0 < self.animation.alpha < 255  and len(stick_anim) == 0:

            for stick in self.sticks:
                if not stick.remove:
                    stick.draw_animation = True
                    break
        for i, stick in enumerate(self.sticks):
            stick: physics.Rectangle
            

                


                

            
                
            if stick.draw_animation:
                if self.animation_num != 0 and self.animation.alpha > 0:
                    self.animation.draw_score_animation(str(self.animation_num))
                   
                    if self.animation.alpha <= 0:
                        stick.draw_animation = False
                        self.animation = ScoreAnimation()
                        self.animation_num = 0
                
                

    def render_polygons(self):
        for poly in self.polygons.copy():
            int_points = []
            if any(math.isnan(vec.x) or math.isnan(vec.y) for vec in poly.world_vertices):
                continue
            if isinstance(poly, physics.Polygon):
                for vec in poly.world_vertices:
                    

                    int_points.append((int(vec.x), int(vec.y)))
                pygame.gfxdraw.aapolygon(self.screen, int_points, (200, 160, 255))
            else:
                for j, child in enumerate(poly.child_polygons):
                    points = [(int(v.x), int(v.y)) for v in child.world_vertices]
                    pygame.gfxdraw.aapolygon(self.screen, points, (200, 160, 255))
                
                
            

            velocity_magnitude = math.sqrt(poly.vx**2 + poly.vy**2 + (poly.length/2 * poly.omega)**2)
            
            pos = pygame.mouse.get_pos()

            same_pos = abs(pos[0] - poly.center_x) < 2 and abs(pos[1] - poly.center_y) < 2
            
            if poly.name != "main_polygon":
                
                if velocity_magnitude < 400 and poly.allow_motion and not same_pos and len(self.polygons) > 1:
                    
                    pass
                    #self.polygons.remove(poly)
                    #man.remove_polygon(poly)
                   # self.index_launch = (self.index_launch - 1) % len(self.polygons)
    def prune_polygons_sticks(self):
        for polygon in self.polygons.copy():
            if polygon.allow_motion:
                if (
                    polygon.center_x > 50000*polygon.length + SCREEN_WIDTH or 
                    
                    polygon.center_y > 50000*polygon.length + SCREEN_HEIGHT or 
                    polygon.center_y < -polygon.length * 50000
                ):
                    if polygon.name != "main_polygon":
                
                        self.polygons.remove(polygon)
                        man.remove_polygon(polygon)
                        self.index_launch = (self.index_launch - 1) % len(self.polygons)
                    else:
                        polygon.center_x = polygon.center_y = 300
        
        for stick in self.sticks.copy():
            stick: physics.Rectangle
            if stick.name != "main_polygon":
                if (
                    stick.center_x > stick.length + SCREEN_WIDTH or 
                    stick.center_x < -stick.length or
                    stick.center_y > stick.length + SCREEN_HEIGHT or 
                    stick.center_y < -stick.length
                ):
                    self.sticks.remove(stick)
                    man.remove_polygon(stick)

    def render_sticks(self):
        for stick in self.sticks.copy():

            stick: physics.Rectangle
            int_points = []
            invalid = False
            for vec in stick.world_vertices:
                if (abs(vec.x) > 1e6 or abs(vec.y) > 1e6) or (math.isnan(vec.x) or math.isnan(vec.y)):
                    self.sticks.remove(stick)
                    invalid = True
                    break
                int_points.append((int(vec.x), int(vec.y)))
            if invalid:
                continue
            if not stick.collided:
                pygame.gfxdraw.filled_polygon(self.screen, int_points, (255, 255, 255))
            else:
                pygame.gfxdraw.filled_polygon(self.screen, int_points, (255, 0, 0))
            if stick.collided:
                stick.allow_motion = True
                self.create_new_stick(stick)
            

            
               
            if stick.remove:
        
                try:
                    self.sticks.remove(stick)
                    man.remove_polygon(stick)
                except ValueError:
                    pass
            squared_velocity = stick.vx * stick.vx + stick.vy * stick.vy + (stick.length/2 * stick.omega)**2
            if squared_velocity < 0:  # just in case
                raise TypeError("NAN")
            velocity_magnitude = math.sqrt(squared_velocity)
            if velocity_magnitude < self.velocity_threshold and stick.allow_motion:
                stick.remove = True
                if stick.draw_animation:
                    stick.draw_animation = False
                    for stick in self.sticks:
                         if not stick.remove:
                            stick.draw_animation = True
                            break
    def endcard_stub(self, clock):
        elapsed_ms = pygame.time.get_ticks() - self.start_time
        remaining_time = max(0, self.countdown_seconds - elapsed_ms // 1000)
        if remaining_time <= 0:
            self.start_time = pygame.time.get_ticks()
            elapsed_ms = pygame.time.get_ticks() - self.start_time
            while elapsed_ms < 2000:
                elapsed_ms = pygame.time.get_ticks() - self.start_time
                self.screen.fill((0,0,0))
                timer_surface = self.font.render(f"Time's up! Your score is: {self.score}", True, (0, 255, 0))
                self.screen.blit(timer_surface, (200, 50)) 
                pygame.display.flip() 
                clock.tick(30) 
            self.score = 0
            self.start_time = pygame.time.get_ticks()
            color = (255, 255, 255)
        elif remaining_time <= 15 and remaining_time > 5:
            color = (255, 255, 0)
            
        elif remaining_time <= 5 and remaining_time < 15:
            color = (255, 0, 0)

        else:
            color = (255, 255, 255)

        timer_surface = self.font.render(f"Seconds Remaining: {remaining_time}", True, color)
        self.screen.blit(timer_surface, (225, 50))
        show_score(self.score, self.screen, self.font)