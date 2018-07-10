# Gravity Gun in Unreal 4

An Unreal 4 project where I recreated the Gravity Gun from Half Life 2 using mostly C++ and some Blueprints (visual scripting)

[Video Reference of the Gravity Gun](https://www.youtube.com/watch?v=T-eJRgJVCZs&feature=youtu.be&t=8m50s)

Classes of importance:

1) GravityGunCharacter - Subclasses from Unreal's Character class and is responsible for the first person movement, controls, camera and gameplay handling. Owns a pointer to a BaseWeapon, which represents the weapon the player is currently holding.
Defines two functions:

	a) OnPrimaryAction()

	b) OnSecondaryAction()

These functions are bound to left click/left trigger and right click/right trigger respectively, and are responsible for calling the underlying functions from the Weapon to respond to the inputs with their behavior

2) BaseWeapon - Subclasses from Unreal's Actor class, and is the base class for all weapons that can be equipped by the GravityGunCharacter. 
Defines two virtual functions:

	a) PrimaryWeaponAction()

	b) SecondaryWeaponAction()

these functions are meant to be implemented by the weapon subclasses based on their gameplay behavior needs.

3) GravityGun - Subclasses from BaseWeapon, and implements the gravity gun behavior. 

Gravity Gun Behavior:

	a) When right click is pressed, any item in range of the player that they are targeting, will be picked up and hovers in front of the player, and moves with them as they move. If right click is pressed again while an object is held, it is dropped gently.

	b) When left click is pressed, any item currently held by the gravity gun or the object in front of the player that is being targeted by the gravity gun will be launched forward with a strong impulse.

