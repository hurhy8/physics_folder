import pygame
class Widget:
    def __init__(self, rect):
        self.rect = pygame.Rect(rect)
        self.hovered = False
        self.active = False

    def draw(self, surface):
        pass

    def handle_event(self, event):
        pass

    def update(self):
        # Can be used for animations or state changes
        pass
class Slider(Widget):
    def __init__(self, rect, min_value, max_value, start_value, callback, 
                 track_color=(255, 255, 255), handle_color=(255, 0, 0), 
                 border_color=(255, 255, 255)):
        super().__init__(rect)
        self.min_value = min_value
        self.max_value = max_value
        self.value = start_value
        self.callback = callback
        self.track_color = track_color
        self.handle_color = handle_color
        self.border_color = border_color
        self.dragging = False
        self.hovered = False

        # Handle dimensions (relative to track)
        self.handle_radius = (self.rect.height // 2) * 2

    def draw(self, surface):
        # Draw track

        
        # Compute handle position
        ratio = (self.value - self.min_value) / (self.max_value - self.min_value)
        self.handle_x = self.rect.x + int(ratio * self.rect.width)
        self.handle_y = self.rect.centery
        rect_before = pygame.Rect(self.handle_x, self.rect.y, self.rect.x - self.handle_x, self.rect.height)
        rect_after = pygame.Rect(
            self.handle_x,
            self.rect.y,
            self.rect.right - self.handle_x,
            self.rect.height
        )
        pygame.draw.rect(surface, self.track_color, rect_after, border_radius=25)
        pygame.draw.rect(surface, self.border_color, rect_after, width=2, border_radius=25)

        pygame.draw.rect(surface, (255,0,0), rect_before, border_radius=25)
        pygame.draw.rect(surface, (255,0,0), rect_before, width=2, border_radius=25)
        
        
        # Draw handle
        pygame.draw.circle(surface, self.handle_color, (self.handle_x, self.handle_y), self.handle_radius)
        
    def handle_event(self, event):
        if event.type == pygame.MOUSEMOTION:
            self.hovered = self.rect.collidepoint(event.pos)
            if self.dragging:
                # Update value based on mouse position
                rel_x = max(0, min(event.pos[0] - self.rect.x, self.rect.width))
                ratio = rel_x / self.rect.width
                self.value = self.min_value + ratio * (self.max_value - self.min_value)
                self.callback(self.value, self)
                
        elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            mouse_x, mouse_y = event.pos
            dist_to_handle = ((mouse_x - self.handle_x)**2 + (mouse_y - self.handle_y)**2) ** 0.5
            if self.rect.collidepoint(event.pos) or dist_to_handle <= self.handle_radius:
                self.dragging = True
                # Update immediately when clicked
                rel_x = max(0, min(event.pos[0] - self.rect.x, self.rect.width))
                ratio = rel_x / self.rect.width
                self.value = self.min_value + ratio * (self.max_value - self.min_value)
                self.callback(self.value, self)
                
        elif event.type == pygame.MOUSEBUTTONUP and event.button == 1:
            self.dragging = False
class Button(Widget):
    def __init__(self, rect, text, callback, font, bg_color=(0, 0, 0), hover_color=(150, 150, 150), border_color = (255,255,255)):
        super().__init__(rect)
        self.text = text
        self.callback = callback
        self.font = font
        self.bg_color = bg_color
        self.hover_color = hover_color
        self.border_color = border_color

    def draw(self, surface):
        color = self.hover_color if self.hovered else self.bg_color
        pygame.draw.rect(surface, color, self.rect)
        pygame.draw.rect(surface, self.border_color, self.rect, width=3, border_radius=3)
        text_surf = self.font.render(self.text, True, (255, 255, 255))
        text_rect = text_surf.get_rect(center=self.rect.center)
        surface.blit(text_surf, text_rect)

    def handle_event(self, event):
        if event.type == pygame.MOUSEMOTION:
            self.hovered = self.rect.collidepoint(event.pos)
        elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.hovered:
                self.callback()
class Label(Widget):
    def __init__(self, rect, text, font, color=(255, 255, 255)):
        super().__init__(rect)
        self.text = text
        self.font = font
        self.color = color
        self.alpha = 255

    def draw(self, screen):
        surface = self.font.render(self.text, True, self.color)
        screen.blit(surface, self.rect.topleft)

    def fade_out(self, screen):
        # Render text
        text_surface = self.font.render(self.text, True, self.color).convert_alpha()

        # Create a transparent surface the same size as the text
        temp_surface = pygame.Surface(text_surface.get_size(), pygame.SRCALPHA)
        
        # Blit the text onto the temp surface with alpha
        text_surface.set_alpha(int(self.alpha))
        temp_surface.blit(text_surface, (0, 0))

        # Blit the temp surface onto the main screen
        screen.blit(temp_surface, self.rect.topleft)

        # Decrease alpha
        self.alpha = max(self.alpha - 2, 0)

class GUIManager:
    def __init__(self):
        self.widgets = []
        self.primitives = []

    def add(self, widget):
        self.widgets.append(widget)
    def add_primitive(self, vertex_points):
        self.primitives.append(vertex_points)
        
    def draw(self, surface):
        for widget in self.widgets:
            widget.draw(surface)
    def draw_polygon(self, surface):
        for vertex_points in self.primitives:
            pygame.draw.polygon(surface, (255, 255, 255), vertex_points, width=1)
    def handle_event(self, event):
        for widget in self.widgets:
            widget.handle_event(event)
    
    def update(self):
        for widget in self.widgets:
            widget.update()
    def kill(self):
        self.widgets = []
    def kill_poly(self):
        self.primitives = []


class Screen:
    def __init__(self):
        self.gui = GUIManager()
    
    def draw(self, surface):
        self.gui.draw(surface)
        self.gui.draw_polygon(surface)
    
    def handle_event(self, event):
        self.gui.handle_event(event)
    
    def update(self):
        self.gui.update()

class ScreenManager:
    def __init__(self):
        self.screen_stack = []

    def push(self, screen):
        self.screen_stack.append(screen)

    def pop(self):
        if len(self.screen_stack) > 1:
            self.screen_stack.pop()

    @property
    def current_screen(self):
        return self.screen_stack[-1]

    def draw(self, surface):
        self.current_screen.draw(surface)

    def handle_event(self, event):
        self.current_screen.handle_event(event)

    def update(self):
        self.current_screen.update()

class CenterMath:
    @staticmethod
    def center(width, height, length_width, length_height):
        
        return  width/2 - length_width/2, height/2 - length_height/2
    @staticmethod
    def center_x(width, length_width):
        return width/2 - length_width/2
    @staticmethod
    def center_y(height, length_height):
        return height/2 - length_height/2