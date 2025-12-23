import pygame

from chatgpt import InputBox 
from game_logic import GameLogic, man, draw_trajectory, Timer
from gui_framework import Label, Button, ScreenManager, Screen, CenterMath
# Initialize Pygame


# Create physics objects
SCREEN_HEIGHT = 900
SCREEN_WIDTH = 1700


FPS = 120









# In your game loop:




input_box = InputBox(100, 100, 140, 32)


    
    




def show_fps(screen, font, clock, polygons, sticks):
    current_fps = f"FPS: {(clock.get_fps()):.2f}"
    fps_surface = font.render(current_fps, True, (255, 255, 255))
    fps_pos = (600, 50)
    screen.blit(fps_surface, fps_pos)
    polygons_surface = font.render(f"amount of objects simulated: {polygons + sticks}", True, (255, 255, 255))
    fps_pos = (800, 50)
    screen.blit(polygons_surface, fps_pos)















def game_loop(game_logic: GameLogic, screen, clock, font, events):
    
    
    for event in events:

        game_logic.handle_event(event)
            

    # UPDATE SECTION
    



    # RENDER SECTION
    screen.fill((0, 0, 0))

    buttons = pygame.mouse.get_pressed()
    
    if buttons[0]:
        for polygon in game_logic.polygons:
            polygon.sleep = False
        game_logic.blitz_logic()
        pos = pygame.mouse.get_pos()
        game_logic.polygon_main.gravity = 0
        game_logic.polygon_main.allow_motion = True

        if game_logic.initial_pos:
            if game_logic.BARRIER - game_logic.initial_pos[0] < 0:...
               # game_logic.draw_barrier()
               # polygon.center_x = game_logic.BARRIER
            elif game_logic.BARRIER - pos[0] < 0:...
                #game_logic.draw_barrier()
                #polygon.center_x = game_logic.BARRIER

        all_polygons_no_motion = [p for p in game_logic.polygons if not p.allow_motion]
        if len(all_polygons_no_motion) >= 0:
            if not game_logic.initial_pos:
                game_logic.initial_pos = pygame.mouse.get_pos()
            draw_trajectory(game_logic.initial_pos, *pos, screen)
    
    
    game_logic.handle_animation_state()
    fractured_pieces = man.step()
    
    if fractured_pieces:
        for fracture in fractured_pieces:
            
            frac1 = fracture[0][0]
            frac1.initialize_self_vector()
            frac2 = fracture[0][1]
            frac2.initialize_self_vector()
            to_remove = fracture[1]
            game_logic.sticks.append(frac1)
            game_logic.sticks.append(frac2)
            if to_remove in game_logic.polygons:
                game_logic.polygons.remove(to_remove)
    game_logic.render_polygons()
    game_logic.render_sticks()


    game_logic.endcard_stub(clock)
    game_logic.prune_polygons_sticks()
    show_fps(screen, font, clock, len(game_logic.polygons), len(game_logic.sticks))



