--
declare extern type mat44 with
    x: vector
    y: vector
    z: vector
    w: vector
end

declare extern type GameObject with
    TransformWS: mat44
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