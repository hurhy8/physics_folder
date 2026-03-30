
import pygame
from pygame.locals import DOUBLEBUF, OPENGL, QUIT
from OpenGL.GL import (
    glEnable, glClear, glLightfv, glBegin, glEnd, glNormal3f, glVertex3f,
    glMaterialfv, GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_DEPTH_TEST,
    GL_LIGHTING, GL_LIGHT0, GL_POSITION, GL_DIFFUSE, GL_SPECULAR, GL_QUADS,
    GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, GL_AMBIENT, GL_SHININESS, glRotatef, glLoadIdentity
)
from OpenGL.GLU import gluPerspective, gluNewQuadric, gluSphere
from OpenGL.GL import glTranslatef

# Initialize Pygame
pygame.init()
display = (800, 600)
pygame.display.set_mode(display, DOUBLEBUF | OPENGL)

# Enable depth testing
glEnable(GL_DEPTH_TEST)

# Set up perspective
gluPerspective(45, (display[0]/display[1]), 0.1, 50.0)
# Move camera back and up
x_translate = 0
y_translate = 0
z_translate = 0

x_rotate = 60
y_rotate = 0
z_rotate = 0


# Enable lighting
glEnable(GL_LIGHTING)
glEnable(GL_LIGHT0)

# White point light above the scene
light_pos = [5.0, 10.0, 5.0, 1.0]  # point light
light_color = [1.0, 1.0, 1.0, 1.0]
ambient_color = [0.3, 0.3, 0.3, 1.0]  # dim ambient
glLightfv(GL_LIGHT0, GL_POSITION, light_pos)
glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color)
glLightfv(GL_LIGHT0, GL_SPECULAR, light_color)
glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_color)

# Set material properties for all objects
material_color = [1.0, 1.0, 1.0, 1.0]
glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material_color)
glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, [50.0])

# Create sphere quadric
quad = gluNewQuadric()

# Drawing functions
def draw_floor():
    glBegin(GL_QUADS)
    glNormal3f(0, 1, 0)
    glVertex3f(-5, 0, -5)
    glVertex3f(5, 0, -5)
    glVertex3f(5, 0, 5)
    glVertex3f(-5, 0, 5)
    glEnd()

def draw_wall(x1, y1, z1, x2, y2, z2, normal):
    glBegin(GL_QUADS)
    glNormal3f(*normal)
    glVertex3f(x1, y1, z1)
    glVertex3f(x2, y1, z2)
    glVertex3f(x2, y2, z2)
    glVertex3f(x1, y2, z1)
    glEnd()

# Main loop
running = True

while running:
    for event in pygame.event.get():
        if event.type == QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_w:
                z_translate += 0.5  # move forward
            elif event.key == pygame.K_s:
                z_translate -= 0.5  # move back


            elif event.key == pygame.K_a:
                x_translate -= 0.5  # move back
            elif event.key == pygame.K_d:
                x_translate += 0.5  # move forward


            elif event.key == pygame.K_q:
                y_translate -= 0.5  # move back

            elif event.key == pygame.K_e:
                y_translate += 0.5  # move forward


            elif event.key == pygame.K_z:
                x_rotate -= 2  # move back

            elif event.key == pygame.K_x:
                x_rotate += 2  # move forward

            elif event.key == pygame.K_c:
                y_rotate -= 2  # move back

            elif event.key == pygame.K_v:
                y_rotate += 2  # move forward



    
    glLoadIdentity()
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    # Camera orientation
    glTranslatef(x_translate, y_translate, z_translate)
    glRotatef(x_rotate, 1, 0, 0)  # pitch
    glRotatef(y_rotate, 0, 1, 0)  # yaw

    glRotatef(z_rotate, 0, 1, 0)  # yaw
    

    # Draw scene
    draw_floor()
    draw_wall(-5, 0, -5, 5, 5, -5, (0, 0, 1))
    draw_wall(-5, 0, 5, 5, 5, 5, (0, 0, -1))
    draw_wall(-5, 0, -5, -5, 5, 5, (1, 0, 0))
    draw_wall(5, 0, -5, 5, 5, 5, (-1, 0, 0))

    glTranslatef(0, 1, 0)
    gluSphere(quad, 1.0, 32, 32)
    glTranslatef(0, -1, 0)

    pygame.display.flip()
    pygame.time.wait(10)
