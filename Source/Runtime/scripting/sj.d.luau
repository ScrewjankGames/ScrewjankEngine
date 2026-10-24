--
declare extern type transform with
    x: vector
    y: vector
    z: vector
    w: vector
    function get_euler_angles_xyz(self): vector
    function get_euler_angles_yxz(self): vector

    function set_rot_euler_xyz(self, eulers: vector)
    function set_rot_euler_zyx(self, eulers: vector)
    function set_rot_euler_yxz(self, eulers: vector)
end

declare extern type GameObject with
    function get_transform_ls(self): transform
    function set_transform_ls(self, lw:transform): ()
    function get_transform_ws(self): transform
    function set_transform_ws(self, lw:transform): ()
end

declare extern type ProcessSignal with
    function Connect(self, fn: (number) -> ())
end

declare extern type InputSystem with
    function GetAxisValue(self, axisName: string): number
    function GetAxisValue2D(self, axisName: string): vector
end

declare Game:
{
    Process: ProcessSignal,
    InputSystem: InputSystem
}