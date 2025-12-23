
import pygame
from hi import game_loop
from game_logic import GameLogic, Timer
from gui_framework import Label, Button, ScreenManager, Screen, CenterMath, Slider
from bowmasters import Play
from enum import Enum, auto
import math
import physics
class GameMode(Enum):
    CIRCLE = auto()
    POLYGON = auto()
    EXOTIC = auto()
running = True
SCREEN_HEIGHT = 900
SCREEN_WIDTH = 1700

timer = Timer()

def create_star(mx, my, length, mass):
    children2 = []
    L = length
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
                length=round(L),
                mass=mass/5,
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

pygame.init()
screen = pygame.display.set_mode((1700, 900))
clock = pygame.time.Clock()
font = pygame.font.SysFont(None, 36)

def compute_length(vertices, base_perimeter):
    return base_perimeter / vertices


def compute_flat_angle(vertices):
    return (math.pi * (vertices - 2))/(2*vertices)

manager = ScreenManager()
class StarScreen(Screen):
    def __init__(self):
        super().__init__()
        self.slider_pair = {}

        self.mass_chosen = 20
        self.length_chosen = 260     # L in your star function
        self.menu_font = pygame.font.SysFont(None, 96)

        # preview star
        self.current_star = self.make_star_preview()
        

    # --------------------------
    # Create star for preview
    # --------------------------
    def make_star_preview(self):
        # center of preview (same as your polygon center)
        mx, my = 800, 450

        # You can modify create_star to accept L/R/mass if needed
        star = create_star(mx, my, self.length_chosen, self.mass_chosen)
        children = (star.child_polygons)
        for child in children:
            coords = [(v.x, v.y) for v in child.world_vertices]
            print(coords)
        return star

    # --------------------------
    # Slider: Mass
    # --------------------------
    def label_change_mass(self, value, slider):
        label = self.slider_pair[slider]
        label.text = f"Mass: {round(value)}"
        w, _ = self.menu_font.size(label.text)
        label.rect.x = CenterMath.center_x(SCREEN_WIDTH * 1.75, w)
        self.mass_chosen = value

        self.update_preview()

    # --------------------------
    # Slider: Length
    # --------------------------
    def label_change_length(self, value, slider):
        label = self.slider_pair[slider]
        label.text = f"Length: {round(value)}"
        w, _ = self.menu_font.size(label.text)
        label.rect.x = CenterMath.center_x(SCREEN_WIDTH * 1.75, w)

        self.length_chosen = value
        self.update_preview()

    # --------------------------
    # Refresh preview
    # --------------------------
    def update_preview(self):
        # If your create_star function later accepts L/mass, plug them in here
        

        self.gui.kill_poly()
        self.current_star = self.make_star_preview()
        self.current_star.center_x = 800
        self.current_star.center_y= 400
        self.current_star.update_world_vertices()
        for child in self.current_star.child_polygons:
            coords = [(v.x, v.y) for v in child.world_vertices]
            self.gui.add_primitive(coords)

    # --------------------------
    # Back button → send star to Character screen
    # --------------------------
    def on_back(self):
        manager.pop()
        for screen in manager.screen_stack:
            if isinstance(screen, Character):
                screen.mass_chosen = self.mass_chosen
                screen.length_chosen = self.length_chosen
                screen.exotic = self.current_star

    # --------------------------
    # Sliders (same as your old system)
    # --------------------------
    def create_slider(self, *args, **kwargs):
        label_text = kwargs["label_text"]
        w, _ = self.menu_font.size(label_text)

        label = Label(
            rect=(kwargs["beginning_x_label"], kwargs["beginning_y_label"],
                  w, kwargs["label_height"]),
            text=label_text,
            font=self.menu_font
        )
        self.gui.add(label)

        slider = Slider(
            rect=(args[0], args[1], kwargs["slider_width"], kwargs["slider_height"]),
            min_value=kwargs["min_value"],
            max_value=kwargs["max_value"],
            start_value=kwargs["start_value"],
            callback=kwargs["callback"]
        )
        self.gui.add(slider)

        self.slider_pair[slider] = label

    # --------------------------
    # Build sliders
    # --------------------------
    def create_star_sliders(self):
        menu_width = 400
        menu_height = 125

        back_x, back_y = CenterMath.center(
            SCREEN_WIDTH * 1.75, SCREEN_HEIGHT / 6, menu_width / 2, menu_height / 2
        )
        self.back = Button(
            rect=(back_x, back_y, menu_width / 2, menu_height / 2),
            text="Back",
            callback=self.on_back,
            font=font
        )
        self.gui.add(self.back)

        slider_width = 400
        slider_height = 30
        slider_x, slider_y = CenterMath.center(
            SCREEN_WIDTH * 1.75, SCREEN_HEIGHT, slider_width, slider_height
        )

        label_height = 125
        padding_label = 90
        padding_slider = 160

        # Mass slider
        w, _ = self.menu_font.size("Mass: 20")
        self.create_slider(
            slider_x, slider_y,
            beginning_x_label=CenterMath.center_x(SCREEN_WIDTH * 1.75, w),
            beginning_y_label=slider_y - padding_label,
            slider_width=slider_width,
            slider_height=slider_height,
            label_height=label_height,
            label_text="Mass: 20",
            min_value=1,
            max_value=150,
            start_value=20,
            callback=lambda v, s: self.label_change_mass(v, s)
        )

        # Length slider
        w2, _ = self.menu_font.size("Length: 260")
        self.create_slider(
            slider_x, slider_y + padding_slider,
            beginning_x_label=CenterMath.center_x(SCREEN_WIDTH * 1.75, w2),
            beginning_y_label=slider_y + padding_slider - padding_label,
            slider_width=slider_width,
            slider_height=slider_height,
            label_height=label_height,
            label_text="Length: 260",
            min_value=120,
            max_value=460,
            start_value=260,
            callback=lambda v, s: self.label_change_length(v, s)
        )
    def create_vertex_options(self):
        x_center = 50
        
        padding = 30
        button_width = 325
        x_center_2 = x_center +button_width + padding
        button_height = 225
        triangle_y = CenterMath.center_y(SCREEN_HEIGHT*0.4, button_height)

       
        shapes = ["Star"]
        y = triangle_y
        x_center = x_center
        for i, shape in enumerate(shapes, start=3):

            btn = Button(
                (x_center, y, button_width, button_height),
                text=shape,
                callback=lambda v=i: self.update_preview(

                ),
                font=self.menu_font
            )
            self.gui.add(btn)
            y += padding + button_height
            
            if (i+ 1) % 3 == 0 and i != 3:
                y = triangle_y
                x_center = x_center + button_width + padding
class PolygonScreen(Screen):
    def __init__(self):
        super().__init__()
        self.slider_pair = {}
        self.mass_chosen = 1
        self.length_chosen = 300
        self.menu_font = pygame.font.SysFont(None, 96)
        self.vertex_chosen = 8
        self.on_vertex_callback(
                    center_x=1000,
                    center_y=450,
                    length=compute_length(self.vertex_chosen, self.length_chosen),
                    angle=compute_flat_angle(self.vertex_chosen),
                    vertices=self.vertex_chosen
                )
    def label_change_mass(self, value, slider_obj):
        label: Label = self.slider_pair[slider_obj]
        label.text = f"Mass: {round(value)}"
        w, h = self.menu_font.size(f"Mass: {round(value)}")
        label.rect.x = CenterMath.center_x(SCREEN_WIDTH*1.75, w)
        self.mass_chosen = value
    def label_change_length(self, value, slider_obj):
        def compute_length(vertices, base_perimeter):
            return base_perimeter / vertices

        
        def compute_flat_angle(vertices):
            return (math.pi * (vertices - 2))/(2*vertices)
        label: Label = self.slider_pair[slider_obj]
        label.text = f"Length: {round(value)}"
        w, h = self.menu_font.size(f"Length: {round(value)}")
        label.rect.x = CenterMath.center_x(SCREEN_WIDTH*1.75, w)
        self.length_chosen = value
        self.on_vertex_callback(
            center_x=1000,
            center_y=450,
            length=compute_length(self.vertex_chosen, self.length_chosen),
            angle=compute_flat_angle(self.vertex_chosen),
            vertices=self.vertex_chosen
        )
    def create_slider(self, beginning_x_slider, beginning_y_slider, beginning_x_label, beginning_y_label, slider_height, slider_width, label_height, label_text, min_value, max_value, start_value, callback):
        

        
        w, h = self.menu_font.size(label_text)
        
        label = Label(rect=(beginning_x_label, beginning_y_label, w, label_height), text=label_text, font=self.menu_font)
        self.gui.add(label)
        slider = Slider(rect=(beginning_x_slider, beginning_y_slider, slider_width, slider_height), min_value=min_value, max_value=max_value, start_value=start_value, callback=callback)
        self.gui.add(slider)
        self.slider_pair[slider] = label
    def on_back(self):
        manager.pop()
        for screen in manager.screen_stack:
            if isinstance(screen, Character):

                screen.mass_chosen = self.mass_chosen
                screen.length_chosen = compute_length(self.vertex_chosen, self.length_chosen)
                
                screen.vertex_chosen = self.vertex_chosen
    def create_polygon_sliders(self):
        menu_width = 400
        menu_height = 125
        back_x, back_y = CenterMath.center(SCREEN_WIDTH*1.75, SCREEN_HEIGHT/6, menu_width/2, menu_height/2)
        self.back =  Button(rect=(back_x, back_y, menu_width/2, menu_height/2), text="Back", callback=self.on_back, font=font)
        self.gui.add(self.back)
        slider_width = 400
        slider_height = 30

        slider_coord_x, slider_coord_y = CenterMath.center(SCREEN_WIDTH*1.75, SCREEN_HEIGHT, slider_width, slider_height)
        label_height = 125
        padding_label = 90
        padding_slider = 160
        w, h = self.menu_font.size("Mass: 1")
        self.create_slider(
            slider_coord_x,
            slider_coord_y,
            beginning_x_label=CenterMath.center_x(SCREEN_WIDTH*1.75, w), 
            beginning_y_label=slider_coord_y - padding_label, 
            
            slider_height=slider_height, 
            slider_width=slider_width, 
            
            label_height=label_height,
            label_text="Mass: 1",
            min_value=1,
            max_value=150,
            start_value=1,
            callback=lambda x, y:self.label_change_mass(x, y)
        )
        w2, h = self.menu_font.size("Length: 200")
        self.create_slider(
            slider_coord_x,
            slider_coord_y + padding_slider,
            beginning_x_label=CenterMath.center_x(SCREEN_WIDTH*1.75, w2), 
            beginning_y_label=slider_coord_y + padding_slider - padding_label, 
            
            slider_height=slider_height, 
            slider_width=slider_width, 
            
            label_height=label_height,
            label_text="Length: 300",
            min_value=300,
            max_value=1000,
            start_value=300,
            callback=lambda x, y:self.label_change_length(x, y)
        )
    

    def on_vertex_callback(self, center_x, center_y, length, angle, vertices):
        self.vertex_chosen = vertices
        self.vertex_points = []
        self.gui.kill_poly()
        radius = length/(2*math.sin(math.pi/vertices))
        for i in range(vertices):
            x = center_x + radius * math.cos((2*math.pi*i)/vertices + angle)
            y = center_y + radius * math.sin((2*math.pi*i)/vertices + angle)
            self.vertex_points.append((x, y))
        self.gui.add_primitive(self.vertex_points)
    def create_vertex_options(self):
        x_center = 50
        
        padding = 30
        button_width = 325
        x_center_2 = x_center +button_width + padding
        button_height = 225
        triangle_y = CenterMath.center_y(SCREEN_HEIGHT*0.4, button_height)

       
        shapes = ["Triangle", "Square", "Pentagon", "Hexagon", "Septagon", "Octagon"]
        y = triangle_y
        x_center = x_center
        for i, shape in enumerate(shapes, start=3):

            btn = Button(
                (x_center, y, button_width, button_height),
                text=shape,
                callback=lambda v=i: self.on_vertex_callback(
                    center_x=1000,
                    center_y=450,
                    length=compute_length(v, self.length_chosen),
                    angle=compute_flat_angle(v),
                    vertices=v
                ),
                font=self.menu_font
            )
            self.gui.add(btn)
            y += padding + button_height
            
            if (i+ 1) % 3 == 0 and i != 3:
                y = triangle_y
                x_center = x_center + button_width + padding


class Character(Screen):
    def __init__(self,perimeter=160, mass_chosen = 1, vertex_chosen=8, exotic=None):
        super().__init__()
        self.menu_font = pygame.font.SysFont(None, 96)
        self.game_chosen = GameMode.CIRCLE
        self.mass_chosen = mass_chosen
        
        self.vertex_chosen = vertex_chosen
        self.exotic = exotic
        self.length_chosen = compute_length(self.vertex_chosen, perimeter)
    def on_back(self):
        manager.pop()
        for screen in manager.screen_stack:
            if isinstance(screen, Menu):

                screen.game_mode = self.game_chosen
                screen.mass = self.mass_chosen
                
                screen.length = self.length_chosen
                screen.vertex_chosen = self.vertex_chosen
                screen.exotic = self.exotic
                
        
    def on_circle_button(self):
        self.game_chosen = GameMode.CIRCLE
        self.circle_mode.bg_color = (150, 150, 150)
        self.circle_mode.hover_color = (150, 150, 150)
        self.polygon_mode.bg_color = (0, 0, 0)
    def on_polygon_button(self):
        self.game_chosen = GameMode.POLYGON
        self.polygon_mode.bg_color = (150, 150, 150)
        self.polygon_mode.hover_color = (150, 150, 150)
        self.circle_mode.bg_color = (0, 0, 0)
        polygon = PolygonScreen()
        manager.push(polygon)
        polygon.create_polygon_sliders()
        polygon.create_vertex_options()
    def on_exotic_button(self):
        self.game_chosen = GameMode.EXOTIC
        self.polygon_mode.bg_color = (150, 150, 150)
        self.polygon_mode.hover_color = (150, 150, 150)
        self.circle_mode.bg_color = (0, 0, 0)
        star = StarScreen()
        manager.push(star)
        star.create_star_sliders()
        star.create_vertex_options()

    def create_buttons_and_labels(self):

    
        menu_width = 400
        menu_height = 125
        padding = 30
        play_x, play_y = CenterMath.center(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, menu_width, menu_height)
        self.circle_mode = Button(rect=(play_x, play_y, menu_width, menu_height), text="Circle mode", callback=self.on_circle_button, font=font)
        self.polygon_mode = Button(rect=(play_x + (menu_width + padding), CenterMath.center_y(SCREEN_HEIGHT/2, menu_height), menu_width, menu_height), text="Polygon Mode", callback=self.on_polygon_button, font=font)
        self.exotic_mode = Button(rect=(play_x + 2*(menu_width + padding), CenterMath.center_y(SCREEN_HEIGHT/2, menu_height), menu_width, menu_height), text="Exotic", callback=self.on_exotic_button, font=font)
        self.gui.add(self.circle_mode)
        self.gui.add(self.polygon_mode)
        self.gui.add(self.exotic_mode)
        back_x, back_y = CenterMath.center(SCREEN_WIDTH/2, SCREEN_HEIGHT/6, menu_width/2, menu_height/2)
        self.back =  Button(rect=(back_x, back_y, menu_width/2, menu_height/2), text="Back", callback=self.on_back, font=font)
        self.gui.add(self.back)
        
        w, h = self.menu_font.size("Character")
        label = Label(rect=(CenterMath.center_x(SCREEN_WIDTH, w), 50, menu_width, menu_height), text="Character", font=self.menu_font)
        self.gui.add(label)
       
    
        
        




class CountdownScreen(Screen):
    def __init__(self, font):
        super().__init__()
        self.timer = Timer()
        w, h = font.size("3!")
        self.label = Label(rect=(CenterMath.center_x(SCREEN_WIDTH, w), (CenterMath.center_y(SCREEN_HEIGHT, h)), 250, 250), text="3!", font=font)
        self.current_number = None

    def update(self, screen):
        count_down = self.timer.count_down(4)

        # Only update text when a new second passes
        if count_down is not None and count_down != self.current_number:
            self.current_number = count_down
            if count_down != 1:
                self.label.text = str(count_down - 1)
            else:
                self.label.text =  "Go!"
            self.label.alpha = 255  # reset fade for new number

        # Draw and fade label
        
        self.label.fade_out(screen)
        return count_down
class Menu(Screen):
    def __init__(self):
        
        super().__init__()
        self.start_game = False
        self.game_mode = GameMode.CIRCLE
        self.mass = 125
        self.length = 400
        self.vertex_chosen = 8
        self.exotic = None
        
    def on_play(self):
        
        manager.pop()
        self.start_game = True

        self.countdown = CountdownScreen(pygame.font.SysFont(None, 356))
        manager.push(self.countdown)
        
        

    def on_character(self):
        char = Character()
        manager.push(char)
        char.create_buttons_and_labels()
    def create_buttons_and_labels(self):

    
        menu_width = 400
        menu_height = 150
        padding = 30
        play_x, play_y = CenterMath.center(SCREEN_WIDTH, SCREEN_HEIGHT, menu_width, menu_height)
        self.play_button = Button(rect=(play_x, play_y, menu_width, menu_height), text="Play", callback=self.on_play, font=font)
        self.character_button = Button(rect=(CenterMath.center_x(SCREEN_WIDTH, menu_width), play_y - (menu_height + padding), menu_width, menu_height), text="Character", callback=self.on_character, font=font)
        self.help_button = Button(rect=(CenterMath.center_x(SCREEN_WIDTH, menu_width), play_y + (menu_height + padding), menu_width, menu_height), text="Help", callback=lambda: print("Placeholder"), font=font)
        self.gui.add(self.play_button)
        self.gui.add(self.character_button)
        self.gui.add(self.help_button)
        menu_font = pygame.font.SysFont(None, 96)
        w, h = menu_font.size("Main Menu")
        label = Label(rect=(CenterMath.center_x(SCREEN_WIDTH, w), 50, menu_width, menu_height), text="Main Menu", font=menu_font)
        self.gui.add(label)

menu = Menu()
manager.push(menu)

menu.create_buttons_and_labels()
instantiation = 0
while running:

    screen.fill((0, 0, 0))

    events = pygame.event.get()
    for event in events:
        if event.type == pygame.QUIT:
            running = False
    if menu.start_game:
        
        
        
        
        #if not menu.countdown.update(screen):
            
        if menu.game_mode.name == "CIRCLE":
            if not instantiation:
                logic = Play()
                
                instantiation +=1
            logic.play(events)
        elif menu.game_mode.name == "POLYGON":
            if not instantiation:
                shape = physics.Polygon(vertices=menu.vertex_chosen, center_x=200, center_y=400, omega=0, mass = round(menu.mass), length=round(menu.length))
                shape.initialize_self_vector()
                game_logic = GameLogic(screen=screen, font=font, shape=shape, anim_num=25000)
                game_logic.initial_pos = game_logic.polygon_main.center_x, game_logic.polygon_main.center_y
                instantiation +=1
            game_loop(game_logic, screen, clock, font, events)
        elif menu.game_mode.name == "EXOTIC":
            if not instantiation:
                shape = menu.exotic
                
                game_logic = GameLogic(screen=screen, font=font, shape=shape, anim_num=25000)
                game_logic.initial_pos = game_logic.polygon_main.center_x, game_logic.polygon_main.center_y
                instantiation +=1
            game_loop(game_logic, screen, clock, font, events)

                
        
        
    else:
        manager.update()
        for event in events:
            if event.type == pygame.QUIT:
                running = False
            manager.handle_event(event)
        
        manager.draw(screen)


    pygame.display.flip()
    clock.tick(220)

pygame.quit()