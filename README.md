# Gravity Gun in Unreal 4

![Gravity Gun](https://user-images.githubusercontent.com/13033542/42537659-5becae8c-844a-11e8-8c32-70813ef5d226.gif)

An Unreal 4 project where I recreated the Gravity Gun from Half Life 2 using mostly C++ and some Blueprints (visual scripting)

[Video Reference of the Gravity Gun](https://www.youtube.com/watch?v=T-eJRgJVCZs&feature=youtu.be&t=8m50s)

Classes of importance:

1) GravityGunCharacter - Subclasses from Unreal's Character class and is responsible for the first person movement, controls, camera and gameplay handling. Owns a pointer to a BaseWeapon, which represents the weapon the player is currently holding.
Defines three important functions:

	a) OnPrimaryAction() - Bound to left click/left trigger, calls ABaseWeapon::PrimaryWeaponAction

	b) OnSecondaryAction() - Bound to right click/right trigger, calls ABaseWeapon::SecondaryWeaponAction
	
	c) OnInteract() - Bound to the keyboard 'E'/ gamepad top face button, allows the player to pickup a nearby weapon or drop a currently equipped weapon
	
	d) PickupWeapon() - Calls ABaseWeapon::OnWeaponPickedUp() 
	
	e) DropWeapon() - Calls ABaseWeapon::OnWeaponDropped()

2) BaseWeapon - Subclasses from Unreal's Actor class, and is the base class for all weapons that can be equipped by the GravityGunCharacter. 
Defines four virtual functions that are meant to be implemented by the weapon subclasses based on their gameplay behavior needs.:

	a) PrimaryWeaponAction()

	b) SecondaryWeaponAction()
	
	c) OnWeaponPickedUp() - Meant to handle any setup a weapon might need when its picked up
	
	d) OnWeaponDropped() - Meant to handle any cleanup of sounds, particles, etc a weapon might need when its dropped

3) GravityGun - Subclasses from BaseWeapon, and implements the gravity gun behavior as follows:

	a) PrimaryWeaponAction() - When left click is pressed, any item currently held by the gravity gun or the object in front of the player that is being targeted by the gravity gun will be launched forward with a strong impulse.

	b) SecondaryWeaponAction() - When right click is pressed, any item in range of the player that they are targeting, will be picked up and hovers in front of the player, and moves with them as they move. If right click is pressed again while an object is held, it is dropped gently.

	c) OnWeaponDropped() - When the gravity gun is dropped, release any currently grabbed objects
	
The C++ classes are all constructed in such a way that they are meant to be subclassed by a Blueprint class in the editor, which allows the user to set properties that require quick changes like meshes, materials, particles, sounds etc through the editor and also avoid direct content references in C++. 

This can be seen in the liberal use of the UPROPERTY() meta specifiers above the member variables of the class, this is how Unreal 4 allows properties to be exposed to the editor UI. 
