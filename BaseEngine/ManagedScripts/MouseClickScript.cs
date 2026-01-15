public class MouseClickScript : Script
{
    public override void Update()
    {
        // Create an InputComponent instance
        InputComponent mouse = new InputComponent(GetEntityID());
        
        // Test 1: Print mouse position every frame
        Debug.WriteLine($"Mouse Position: X={mouse.MouseX}, Y={mouse.MouseY}");
        
        // Test 2: Check if left mouse button (0) is clicked
        if (mouse.IsButtonTriggered(0))
        {
            Debug.WriteLine($"Left Click at World Pos: X={mouse.WorldMouseX}, Y={mouse.WorldMouseY}");
        }
        
        // Test 3: Check if mouse is in game viewport
        if (mouse.InGameViewport)
        {
            Debug.WriteLine("Mouse is in game viewport");
        }
    }
}