import pygame
SCREEN_WIDTH, SCREEN_HEIGHT = 1700, 900
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))


class ScoreAnimation:
    def __init__(self):
        self.font = pygame.font.SysFont("arial", 195)
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
        self.v_gradients = {}
        self.h_gradients = {}

        for value, color in self.color_dict.items():
            self.v_gradients[color] = self.make_vertical_gradient(
                30, 125, (0, 0, 0), color
            )
            self.h_gradients[color] = self.make_horizontal_gradient(
                125, 30, (0, 0, 0), color
            )
        
        self.animation1 = 150
        self.animation2 = 197.5
        self.animation3 = 150
        self.step = 0.009
        self.fade = False
        
        self.alpha = 255
       
        self.surf = pygame.Surface((1700, 900), pygame.SRCALPHA)
        self.surf.fill((0, 0, 0, 0))   
        self.blitz_animation_x = 2
        self.blitz_animation_y = 45
    def round_to_five(self, num):
        return self.color_dict[min(int(num) - (int(num) % 5), 45)]


    def make_vertical_gradient(self, width, height, top_color, bottom_color):
        surf = pygame.Surface((width, height), pygame.SRCALPHA)
        for y in range(height):
            ratio = y / height
            r = int(top_color[0] * (1 - ratio) + bottom_color[0] * ratio)
            g = int(top_color[1] * (1 - ratio) + bottom_color[1] * ratio)
            b = int(top_color[2] * (1 - ratio) + bottom_color[2] * ratio)
            pygame.draw.line(surf, (r, g, b, 255), (0, y), (width, y))
        return surf
    def make_horizontal_gradient(self, width, height, top_color, bottom_color):
        surf = pygame.Surface((width, height), pygame.SRCALPHA)
        for y in range(width):
            ratio = y / width
            r = int(top_color[0] * (1 - ratio) + bottom_color[0] * ratio)
            g = int(top_color[1] * (1 - ratio) + bottom_color[1] * ratio)
            b = int(top_color[2] * (1 - ratio) + bottom_color[2] * ratio)
            pygame.draw.line(surf, (r, g, b, 255), (y, 0), (y, height))
        return surf
   
    def fade_out(self, num):
        


        rect = pygame.Rect(SCREEN_WIDTH/2 - 15, self.animation1, 30, 125)
        
        grad = self.v_gradients[self.round_to_five(num)]
        self.surf.blit(grad, rect.topleft)
        rect2 = pygame.Rect(SCREEN_WIDTH/2 - 125/2, self.animation2, 125, 30)
        grad2 = self.h_gradients[self.round_to_five(num)]
        self.surf.blit(grad2, rect2.topleft)
        self.text_surface.set_alpha(self.alpha)
        
        if self.alpha <= 0:
            self.alpha = 0
        else:
            
            self.alpha -= 2.6
            self.alpha = max(self.alpha, 0)
        screen.blit(self.text_surface, (SCREEN_WIDTH/2 - 125/2 + 150, self.animation3/2))
        screen.blit(self.surf, (0, 0))
        self.surf.fill((0, 0, 0, 0))
    
    def draw_score_animation(self, num):
        
        self.text_surface = self.font.render(num, True, self.round_to_five(num))
        #print(self.animation1)
        if not self.fade:
            if self.animation1 <= 50 and self.animation2 <= 97.5:
                self.animation1 = 50
                self.animation2 = 97.5   
                self.animation3 = 50
                self.fade = True
            rect = pygame.Rect(SCREEN_WIDTH/2 - 15, self.animation1, 30, 125)
            grad = self.v_gradients[self.round_to_five(num)]
            self.surf.blit(grad, rect.topleft)
            
            rect2 = pygame.Rect(SCREEN_WIDTH/2 - 125/2, self.animation2, 125, 30)
            grad2 = self.h_gradients[self.round_to_five(num)]
            self.surf.blit(grad2, rect2.topleft)
            self.animation3 -= 14
            
            self.animation1 -= 14
            self.animation2 -=  14
        else:
            self.fade_out(num)
        
    
        # Draw it on screen
        screen.blit(self.text_surface, (SCREEN_WIDTH/2 - 125/2 + 150, self.animation3/2))
        screen.blit(self.surf, (0, 0))
        self.surf.fill((0, 0, 0, 0))
    def draw_blitz_mode(self):
        if self.blitz_animation_x >= 195 and self.blitz_animation_y >= SCREEN_WIDTH - 195:
            self.blitz_animation_x = 2
            self.blitz_animation_y = SCREEN_WIDTH - 195
        
        self.text_surface = self.font.render("Blitz Mode", True, (255, 255, 0))
        screen.blit(self.text_surface, (self.blitz_animation_x, self.blitz_animation_y))
        self.blitz_animation_x = self.blitz_animation_x * 1.1
        
        