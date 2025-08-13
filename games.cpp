// games.cpp: A program using the TL-Engine

#include "TL-Engine11.h" // TL-Engine11 include file and namespace
using namespace tle;

//reference to function for collision detection
//bool sphere2sphere(IModel* cube, IModel* marble, float radius1, float radius2);

bool Sphere2Box(IModel* Cube, IModel* Marble, float mRadius, float cWidth, float cDepth);

//enum cubeSide { leftside, rightside, frontside, backside, noside };
//cubeSide enhancedSphere2Box(IModel* Cube, IModel* Marble, float mRadius, float cWidth, float cDepth, float oldMXPos, float oldMZPos);

int main()
{
	// Create a 3D engine (using TL11 engine here) and open a window for it
	TLEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\Uni Work\\Uni Work Year 1\\CO1301 - Games Concepts\\Assignment 1 - 7\\");

	/**** Set up your scene here ****/
	const float kGameSpeed = 2.0f;             // Speed of the game //at three for testing
	const float kRotationSpeed = 0.5f;
	const float kMaxTravelDistance = 200.0f;
	const float marbleRadius = 3.25f;
	const float cubeRadius = 6.0f;
	const float cubeWidth = 4.0f;              // for sphere2box
	const float cubeDepth = 4.0f;
	

	const int kBarrierArraySize = 20;          // sets the amount of barriers in the array
	const int resetBarrierZ = 18;              //for alligning the barriers
	const int halfbarrierSize = kBarrierArraySize / 2;

	int numberOfBlocks = 10;                   // number of blocks, this is used for the collision detection bit

	float kMarbleSpeed = (0.1f * kGameSpeed); // the initial marble speed
	float dummyRotation = 0.1f;               // dummy roataion for arrow control
	float cubeSize = 8.0f;
	float barrierZ = 18;                      // initialize gap between barrier
	float barrierX = 0;                       // initialize the X for the barriers
	float gapBetweenBarriers = 18.0f;
	float fiveCubeHits = 5;
	float arrowCapRight = -50.0f;
	float arrowCapLeft = 50.0f;
	float barrierLineLeft = 67.0f;
	float barrierLineRight = -67.0f;
	float lastFour = 4;

	float marbleDirectionX;                   // float for marbles X direction, to use for bouncing in realistic directions *
	float marbleDirectionZ;                   // *

	                                          // creates camera
	ICamera* myCamera = myEngine->CreateCamera(kManual, 0.0f, 30.0f, -60.0f);
	myCamera->RotateLocalX(10.0f);            // tilts camera down 

	IMesh* barrierMesh = myEngine->LoadMesh("Barrier.x");
	IModel* barrier[kBarrierArraySize];       // array of barrier models

	IMesh* floorMesh = myEngine->LoadMesh("Floor.x");
	IModel* floor = floorMesh->CreateModel(0.0f, 0.0f, 0.0f);

	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox_Hell.x");
	IModel* skybox = skyboxMesh->CreateModel(0.0f, -1000.0f, 0.0f);

	IMesh* arrowMesh = myEngine->LoadMesh("Arrow.x");
	IModel* arrow = arrowMesh->CreateModel(0.0f, 5.0f, -10.0f);
	arrow->MoveZ(-10.0f);                     // moves the arrow on the Z axis by -10.0f
	arrow->MoveY(2.3f);                       // moves the arrow on the Y axis by 2.3f

	float initialMarbleX = 0.0f;              // var for the first marble X , Y and Z
	float initialMarbleY = 2.0f;
	float initialMarbleZ = 0.0f;
	IMesh* marbleMesh = myEngine->LoadMesh("Marble.x");
	IModel* marble = marbleMesh->CreateModel(initialMarbleX, initialMarbleY, initialMarbleZ);

	IMesh* dummyMesh = myEngine->LoadMesh("Dummy.x");
	IModel* dummy = dummyMesh->CreateModel(initialMarbleX, initialMarbleY, initialMarbleZ);
	
	const float arrowOffsetZ = -10.0f;
	arrow->AttachToParent(dummy);
	arrow->SetLocalPosition(0.0f, 0.0f, arrowOffsetZ);

	const int kCubeArraySize = 10;
	IMesh* cubeMesh = myEngine->LoadMesh("Cube.x");
	IModel* cube[kCubeArraySize]; // creates the array

	/*float oldXPos = marble->GetX(); //was gonna use this for enhancedSphere2box
	float oldZPos = marble->GetZ();*/

	// create cube models and position them and adds a gap inbetween them all
	float gapBetweenCubes = 12.0f;
	float cubeXpos = -54.0f;
	for (int i = 0; i < kCubeArraySize; i++)
	{
		cube[i] = cubeMesh->CreateModel(cubeXpos, 5.0f, 120.0f); // sets the X,Y,Z position for all cubes in the array
		cubeXpos += gapBetweenCubes; //basically adds whatever the value of gapbetweenCubes is, to the value of cubeXpos
	}

	//LEFT SIDE BARRIERS
	float leftBarrierX = -70.0f; //this moves barriers to left
	for (int i = 0; i < halfbarrierSize; i++)
	{
		barrier[i] = barrierMesh->CreateModel(leftBarrierX, 0.0f, barrierZ); //creates barrier model

		if (i >= (halfbarrierSize) - lastFour) // this finds the last 4 and changes the skin to stripey
		{
			barrier[i]->SetSkin("barrier1.bmp");
		}
		else
		{
			barrier[i]->SetSkin("barrier1a.bmp"); //barriers before the last 4
		}

		barrierZ += gapBetweenBarriers; //creates the gap between barriers 
	}

	barrierZ = resetBarrierZ; // resets the barrier Z postion so the barriers on the right arent far away.

	//RIGHT SIDE BARRIERS
	float rightBarrierX = 70.0f; // this moves barriers to right
	for (int i = 0; i < halfbarrierSize; i++) 
	{
		barrier[i] = barrierMesh->CreateModel(rightBarrierX, 0.0f, barrierZ);

		if (i >= (halfbarrierSize) - lastFour) //if i is greater than or equal to half the barriers array size and minus 4, make skin barrier1.bmp  
		{
			barrier[i]->SetSkin("barrier1.bmp");
		}
		else
		{
			barrier[i]->SetSkin("barrier1a.bmp");
		}

		barrierZ += gapBetweenBarriers;
	}

	// enums for each game state
	enum currentState { Ready, Firing, Contact, Over };
	currentState gamesState = Ready;
                                               // array to keep track of cube contact or barrier contact
	bool cubeContact[10]; //initialise a boolean array, to represent the 10 contacted cubes or 20 barriers 
	/*bool barrierContact[20];*/
	                                           
	for (int i = 0; i < numberOfBlocks; i++)   ///////
	{
		cubeContact[i] = false;                //sets contact to cubes as false
	}

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene(); // do it like 4 if statments, use all the states.

		/**** Update your scene each frame here ****/

		// if statment for when the state is in the Ready phase. 
		if (gamesState == Ready)
		{
			if (myEngine->KeyHeld(Key_Z) && dummyRotation >= arrowCapRight) //puts a cap on how far the arrow/marble can be aimed. *
			{
				marble->RotateLocalY(kRotationSpeed * kGameSpeed); // if key Z is held, and dummy rotation is greater than or equal to -50.0f
				dummy->RotateLocalY(kRotationSpeed * kGameSpeed);  // then rotate the marble and dummy by the sum of the roatation speed and the game speed after being times togetherr.
				dummyRotation -= (kRotationSpeed * kGameSpeed);       
			}
			if (myEngine->KeyHeld(Key_X) && dummyRotation <= arrowCapLeft) // *
			{
				marble->RotateLocalY(-kRotationSpeed * kGameSpeed);
				dummy->RotateLocalY(-kRotationSpeed * kGameSpeed);
				dummyRotation += (kRotationSpeed * kGameSpeed);
			}
			if (myEngine->KeyHit(Key_Space))
			{
				gamesState = Firing;                        // change the games state to firing if space has been hit.

				float startMarbleX = marble->GetX();        //
				float startMarbleZ = marble->GetZ();

				marble->MoveLocalZ(1);                    // move the marble along its Z 
				float movedMarbleX = marble->GetX();       //store the X coordinate
				float movedMarbleZ = marble->GetZ();      // store the Z coordinate **

				marble->MoveLocalZ(-1);
				marbleDirectionX = movedMarbleX - startMarbleX;  //**
				marbleDirectionZ = movedMarbleZ - startMarbleZ;
			}
		}
		// change to firing
		if (gamesState == Firing)
		{
			//Move the marble forward on Z axis 
			float currentZ = marble->GetZ();

			/*marble->MoveLocalZ(kMarbleSpeed);*/

			marble->MoveX(marbleDirectionX * kMarbleSpeed);
			marble->MoveZ(marbleDirectionZ * kMarbleSpeed);

			// sets the arrow position
			arrow->SetLocalPosition(0.0f, 0.0f, arrowOffsetZ);

			// collision checking with cubes
			for (int i = 0; i < kCubeArraySize; i++)
			{
				if (Sphere2Box(marble, cube[i], marbleRadius, cubeWidth, cubeDepth)) 
				{
					cube[i]->SetSkin("tiles_red.jpg");
					cubeContact[i] = true;

					// reverse marble speed on collision with cubes.
					marbleDirectionZ = -marbleDirectionZ;

					// change state to contact
					gamesState = Contact;
				}
			}

			//this below is code for moving the arrow whilst in the firing state
			if (myEngine->KeyHeld(Key_Z) && dummyRotation >= arrowCapRight)
			{
				dummy->RotateLocalY(kRotationSpeed * kGameSpeed);
				dummyRotation -= (kRotationSpeed * kGameSpeed);
			}
			if (myEngine->KeyHeld(Key_X) && dummyRotation <= arrowCapLeft)
			{
				dummy->RotateLocalY(-kRotationSpeed * kGameSpeed);
				dummyRotation += (kRotationSpeed * kGameSpeed);
			}

			//makes the ball bounce off the barriers into the cubes and eventually back to the arrow.
			if (marble->GetX() >= barrierLineLeft)
			{
				marbleDirectionX = -marbleDirectionX;  //reverse direciton
			}
			else if (marble->GetX() <= barrierLineRight)        
			{
				marbleDirectionX = -marbleDirectionX;
			}

			if (marble->GetZ() > kMaxTravelDistance)
			{
				kMarbleSpeed = -(kMarbleSpeed);  //when the marble misses the cubes and hits the kMaxTravelDistance, it snaps back into place at its initial X,Y,Z
				marble->SetPosition(initialMarbleX, initialMarbleY, initialMarbleZ);
			}

			//without this, the marble will fly past the arrow after bouncing back off the cubes.
			if (marble->GetZ() < initialMarbleZ)
			{
				marble->ResetOrientation(); //resets where it is to where it was.
				kMarbleSpeed = -(kMarbleSpeed);
				gamesState = Ready;
				marble->SetPosition(initialMarbleX, initialMarbleY ,initialMarbleZ);  // basically snaps the marble back in place after reaching all initial X,Y,Z points.

				marble->RotateY(-dummyRotation); //rotates the marble back so its facing the way the dummy is to be fired the correct direction.
			}
			
			//if the currentZ value 
			if (currentZ + kMarbleSpeed >= kMaxTravelDistance)
			{
				// Stop the marble
				marble->SetLocalPosition(initialMarbleX, initialMarbleY, kMaxTravelDistance);
			}
		}

		if (gamesState == Contact)
		{
			bool collision = false;

			// var for keeping track of the number of hits on cubes
			int numberOfHits = 0;

			for (int i = 0; i < numberOfBlocks; i++) //for loop for the blocks contact
			{
				if (cubeContact[i] == true)
				{
					numberOfHits++; //increment the number of hits
				}
			}

			if (numberOfHits == fiveCubeHits) //transition to over state if 5 blocks have been hit
			{
				gamesState = Over;
				marble->SetSkin("glass_green.jpg");
			}
			else
			{
				gamesState = Firing; //transition back to firing if game isnt over yet
			}
		}

		if (gamesState == Over)
		{
			marble->SetSkin("glass_green.jpg");
		}

		// Stop if the Escape key is pressed
		if (myEngine->KeyHit(Key_Escape))
		{
			myEngine->Stop();
		}
	}
	myEngine->Delete();
}

//bool sphere2sphere(IModel* cube, IModel* marble, float radius1, float radius2)
//{
//	float distX = marble->GetX() - cube->GetX(); //distance between the marbles X taken from the cubes X, **
//	float distZ = marble->GetZ() - cube->GetZ(); //**
//	float  dist = sqrt((distX * distX) + (distZ * distZ));
//
//	return dist < (radius1 + radius2);
//}

bool Sphere2Box(IModel* Cube, IModel* Marble, float mRadius, float cWidth, float cDepth)
{
	float minX = Cube->GetX() - (cWidth / 2) - mRadius; //subtracts half the width then subtracts the radius to find the minX
	float maxX = Cube->GetX() + (cWidth / 2) + mRadius; //adds half the width then adds the radius to find the maxX etc etc 
	float minZ = Cube->GetZ() - (cDepth / 2) - mRadius; 
	float maxZ = Cube->GetZ() + (cWidth / 2) + mRadius;
	
	return (Marble->GetX() > minX && Marble->GetX() < maxX // works out if the marble is in the cube
		&& Marble->GetZ() > minZ && Marble->GetZ() < maxZ);
}

//cubeSide enhancedSphere2Box(IModel* Cube, IModel* Marble, float mRadius, float cWidth, float cDepth, float oldMXPos, float oldMZPos)
//{
//	cubeSide result = noside;
//
//	float minX = Cube->GetX() - (cWidth / 2) - mRadius; //subtracts half the width then subtracts the radius to find the minX
//	float maxX = Cube->GetX() + (cWidth / 2) + mRadius; //adds half the width then adds the radius to find the maxX etc etc 
//	float minZ = Cube->GetZ() - (cDepth / 2) - mRadius;
//	float maxZ = Cube->GetZ() + (cWidth / 2) + mRadius;
//
//	if (Marble->GetX() > minX && Marble->GetX() < maxX // works out if the marble is in the cube
//		&& Marble->GetZ() > minZ && Marble->GetZ() < maxZ)
//	{
//		if (oldMXPos <= minX) result = leftside;
//		else if (oldMXPos >= maxX) result = rightside;
//		else if (oldMZPos <= minZ) result = frontside;
//		else result = backside;
//	}
//	return result;
//}