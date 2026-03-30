from typing import List, Tuple, Optional, Dict, Any

class Vec2:
    x: float
    y: float
    def __init__(self, x: float = 0, y: float = 0) -> None: ...
class AABB:
    min_x: float
    max_x: float
    min_y: float
    max_y: float
    def __init__(self, min_x: float = 0, max_x: float = 0, min_y: float = 0, max_y: float = 0) -> None: ...
class Shape:
    sleep: bool
    num_times_sleepy: int
    vx: float
    vy: float
    center_x: float
    center_y: float
    length: float
    omega: float
    angle: float
    gravity: float
    mass: float
    elasticity: float
    collision_elasticity: float
    allow_motion: bool
    collision_frame_handled: bool
    draw_animation: bool
    num_times_sleepy: int
    collided: bool
    remove: bool
    edge_coordinates: List[Tuple[Vec2, Vec2]]
    name: str
    

    def __init__(self, vx: float = 25, center_x: float = 400, center_y: float = 0,
                 length: float = 110, omega: float = 3, angle: float = 0, mass: float = 25, static: bool = False) -> None: ...

class Polygon(Shape):
    next_id: int
    id: int
    vertices: int
    sleep: bool
    inertia: float
    world_vertices: List[Vec2]
    aabb: AABB

    def __init__(self, vertices: int = 4, vx: float = 0, center_x: float = 400,
                 center_y: float = 0, length: int = 110, omega: float = -3,
                 angle: float = 0, mass: float = 25) -> None: ...

    def sat_algorithm_boundary(self, segment_edge: Vec2, segment_vertices: Tuple[Vec2, Vec2]) -> Tuple[bool, Vec2]: ...
    def initialize_self_vector(self) -> None: ...
    def wall(self, segv1: Vec2, segv2: Vec2) -> None: ...
    def clamp_motion(self) -> None: ...
    def is_flat(self, angle: float, flat_angles: List[float], tolerance: float = 0.001) -> bool: ...
    def put_to_sleep(self) -> None: ...
    def impulse_resolution(self, MTV: Vec2, segment_edge: Vec2, segment_vertices: Tuple[Vec2, Vec2]) -> None: ...
    def update_world_vertices(self) -> None: ...
    def apply_motion(self, dt: float) -> None: ...
    def apply_rotation(self, lever_arm_r: Vec2, Jx: float, Jy: float) -> None: ...
    def AABB_overlap(self, a: Polygon, b: Polygon):...

    # private members
    local_center_x: float
    local_center_y: float
    local_vertices: List[Vec2]
    def calculate_vertices_and_inertia(self) -> List[Vec2]: ...
    def compute_line_segment_projection(self, segment_vertices: Tuple[Vec2, Vec2], normal: Vec2) -> Tuple[float, float]: ...
    def closest_point_on_segment(self, seg_vert_1: Vec2, seg_vert_2: Vec2, P: Vec2) -> Vec2: ...
    def support_vertices(self, seg_vert_1: Vec2, seg_vert_2: Vec2, seg_normal: Vec2, edge_coordinates: List[Tuple[Vec2, Vec2]]) -> List[Vec2]: ...
    def build_contact_manifold(self, seg_a: Vec2, seg_b: Vec2, seg_normal: Vec2, polygon_edges: List[Tuple[Vec2, Vec2]], max_contacts: int, tolerance: float = 1e-6) -> List[Vec2]: ...

class PolygonCollisions:
    restitution: float
    impulse_cache: Dict[Tuple[int, int, float], Tuple[float, float]]

    def __init__(self) -> None: ...
    def normal_computation(self, polygon: Polygon) -> List[Vec2]: ...
    def poly_min_max(self, polygon: Polygon, normal: Vec2) -> Tuple[float, float]: ...
    def sat_algorithm(self, polygon: Polygon, polygon_other: Polygon) -> Tuple[bool, Vec2, Vec2, List[Vec2]]: ...
    def impulse_resolution(self, polygon: Optional[Polygon], polygon_other: Optional[Polygon], contact: Vec2,
                           MTV: Vec2, iterations: int, penn_corr: float = 1.0/12.5) -> Tuple[float, float]: ...
    def apply_impulse(self, polygon: Optional[Polygon], polygon_other: Optional[Polygon], contact: Vec2, impulses: Tuple[float, float]) -> None: ...
    def polygon_collisions(self, polygons: List[Optional[Polygon]]) -> None: ...

    # private methods
    def segments_intersect(self, p1: Vec2, p2: Vec2, q1: Vec2, q2: Vec2) -> bool: ...
    def line_intersects_segment(self, begin_point: Vec2, end_point: Vec2, polygon: Polygon) -> int: ...
    def raycaster(self, point: Vec2, polygon: Polygon, ray_length: int = 200000) -> int: ...
    def polygon_contact_points(self, polygon: Polygon, polygon_other: Polygon) -> List[Vec2]: ...
    def relative_velocity(self, polygon: Polygon, contact: Vec2) -> Tuple[float, float, Vec2]: ...

class Rectangle(Polygon):
    def calculate_vertices_and_inertia(self) -> List[Vec2]: ...
    def put_to_sleep(self) -> None: ...
    def __init__(self, vertices: int = 4, vx: float = 0, center_x: float = 400,
                 center_y: float = 0, length: int = 110, width=70, omega: float = 3,
                 angle: float = 0, mass: float = 25) -> None: ...
class IsoscelesTriangle(Polygon):
    def calculate_vertices_and_inertia(self) -> List[Vec2]: ...
    def put_to_sleep(self) -> None: ...
    def __init__(self, vertices: int = 4, vx: float = 0, center_x: float = 400,
                 center_y: float = 0, equal_side: int = 110, unique_side=70, omega: float = 3,
                 angle: float = 0, mass: float = 25) -> None: ...
class PolygonManager:
    polygons: List[Optional[Polygon]]
    collision_class: Optional[PolygonCollisions]

    def __init__(self, collision_class: Optional[PolygonCollisions]) -> None: ...
    def add_polygon(self, poly: Optional[Polygon]) -> None: ...
    def remove_polygon(self, poly: Optional[Polygon]) -> None: ...
    def step(self) -> Optional[List[Tuple[Tuple, Polygon]]]: ...


class CompoundShape:
    # Properties
    child_polygons: List[Polygon]
    world_vertices: List[Vec2]
    edge_coordinates: List[Tuple[Vec2, Vec2]]
    vertices: int
    aabb: AABB
    center_x: float
    center_y: float
    local_center_x: float
    local_center_y: float
    vx: float
    vy: float
    omega: float
    mass: float
    inertia: float
    collision_elasticity: float
    length: float
    sleep: bool
    allow_motion: bool
    num_times_sleepy: int
    gravity: float
    angle: float




class PendulumJoint:
    rigid_body: Shape
    body_anchor: Vec2
    world_anchor: Vec2
    rope_length: float

    def __init__(
        self,
        rigid_body: Shape,
        body_anchor: Vec2,
        world_anchor: Vec2,
        rope_length: float,
    ) -> None: ...

    def update_local_anchor(self) -> Vec2: ...

class PendulumJoint:
    
    body_anchor: Vec2
    world_anchor: Vec2
    rigid_body: Shape
    rope_length: float

    def __init__(
        self,
        
        body_anchor: Vec2,
        world_anchor: Vec2,
        rigid_body: Shape
    ) -> None: ...

    def update_local_anchor(self) -> Vec2: ...




class DynamicJoint:
    body_anchor1: Vec2
    body_anchor2: Vec2
    rope_length: float
    initial_components: Vec2
    rigid_body1: Shape
    rigid_body2: Shape

    def __init__(
        self,
        anchor1: Vec2,
        anchor2: Vec2,
        body: Shape,
        body_other: Shape
    ) -> None: ...
    
    def impulse_resolution(self) -> None: ...
    
    def relative_velocity(self, polygon: Shape, contact: Vec2) -> Tuple[float, float, Vec2]: ...
    
    def update_local_anchor(self, anchor: Vec2, shape: Shape) -> Vec2: ...
    
    def getRopeLength(self, body_to_world_anchor1: Vec2, body_to_world_anchor2: Vec2) -> float: ...


class SoftJoint:
    body_anchor1: Vec2
    body_anchor2: Vec2
    k: float
    rope_length: float
    initial_components: Vec2
    rigid_body1: Shape
    rigid_body2: Shape

    def __init__(self, 
                 world1: Vec2, 
                 world2: Vec2, 
                 body: Shape, 
                 body_other: Shape, 
                 k_: float) -> None: ...
    
    def impulse_resolution(self) -> None: ...
    
    def relative_velocity(self, polygon: Shape, contact: Vec2) -> Tuple[float, float, Vec2]: ...
    
    def update_local_anchor(self, anchor: Vec2, shape: Shape) -> Vec2: ...
    
    def getRopeLength(self, body_to_world_anchor1: Vec2, body_to_world_anchor2: Vec2) -> float: ...
class PendulumSolver:
    joints: List
    def __init__(self) -> None: ...

    def add_joint(self, joint: PendulumJoint) -> None: ...

    def solve(self) -> None: ...

    def remove_rigid_body(self, shape: Shape) -> None: ...