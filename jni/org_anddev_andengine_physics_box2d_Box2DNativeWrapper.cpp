#include "org_anddev_andengine_physics_box2d_Box2DNativeWrapper.h"
#include "box2d-2.0.1/Include/Box2D.h"
#include "stdlib.h"
#include "stdio.h"
#include <jni.h>
#include <android/log.h> 

#define LOG_TAG "AndEngine-Native"

const int32 MAX_BODIES = 300;

int32 BODYINDEX = 0;
b2Body *BODIES[MAX_BODIES];
bool BODIES_CONTACT_ENABLED[MAX_BODIES];

b2World *WORLD = NULL;

struct BodyIndexBodyData {
   int32 bodyIndex;
};

class JNIProxyContactListener : public b2ContactListener {
private:
	JNIEnv* mEnv;
	jobject mContactListener;
	jmethodID mAddMethodID;

public:
	void Add(const b2ContactPoint* pContactPoint) {
		if(this->mContactListener != NULL) {
			int32 physicsIDA = ((BodyIndexBodyData*)pContactPoint->shape1->GetBody()->GetUserData())->bodyIndex;
			int32 physicsIDB = ((BodyIndexBodyData*)pContactPoint->shape2->GetBody()->GetUserData())->bodyIndex;
			if(BODIES_CONTACT_ENABLED[physicsIDA] || BODIES_CONTACT_ENABLED[physicsIDB]) {
				this->mEnv->CallVoidMethod(this->mContactListener, this->mAddMethodID, physicsIDA, physicsIDB);
			}
		}
	}

	void SetEnv(JNIEnv* pEnv, jobject pContactListener) {
		this->mEnv = pEnv;
		this->mContactListener = pContactListener;
		
		if(pContactListener != NULL) {
			jclass contactListenerClass = this->mEnv->GetObjectClass(this->mContactListener);
			this->mAddMethodID = this->mEnv->GetMethodID(contactListenerClass, "add", "(II)V");
		}
	}

	void Persist(const b2ContactPoint* pContactPoint) {
		// handle persist point
	}

	void Remove(const b2ContactPoint* pContactPoint) {
		// handle remove point
	}

	void Result(const b2ContactResult* pContactResult) {
		// handle results
	}
};

JNIProxyContactListener *JNI_PROXY_CONTACTLISTENER;

extern "C" {
	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    createWorld
	 * Signature: (FFFFFF)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_createWorld (JNIEnv* pEnvironment, jobject pCaller, jfloat pMinX, jfloat pMinY, jfloat pMaxX, jfloat pMaxY, jfloat pGravityX, jfloat pGravityY){
		if(WORLD != NULL){
			/* Clear the WORLD. */
			for(int32 i = 0; i < MAX_BODIES; i++){
				if(BODIES[i] != NULL){
					WORLD->DestroyBody(BODIES[i]);
					BODIES[i] = NULL;
				}
			}
			WORLD = NULL;
			BODYINDEX = 0;
		}

		b2AABB worldAABB;
		worldAABB.lowerBound.Set(pMinX, pMinY);
		worldAABB.upperBound.Set(pMaxX, pMaxY);

		// Define the gravity vector.
		b2Vec2 gravity(pGravityX, pGravityY);

		// Do we want to let bodies sleep?
		bool doSleep = false;

		// Construct a world object, which will hold and simulate the rigid bodies.
		WORLD = new b2World(worldAABB, gravity, doSleep);
		
		JNI_PROXY_CONTACTLISTENER = new JNIProxyContactListener();
		WORLD->SetContactListener(JNI_PROXY_CONTACTLISTENER);
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    step
	 * Signature: (FII)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_step (JNIEnv *pEnvironment, jobject pCaller, jobject pContactListener, jfloat pTimeStep, jint pIterations){
		JNI_PROXY_CONTACTLISTENER->SetEnv(pEnvironment, pContactListener);
		WORLD->Step(pTimeStep, pIterations);
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    createCircle
	 * Signature: (FFFFF)I
	 */
	JNIEXPORT jint JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_createCircle (JNIEnv* pEnvironment, jobject pCaller, jfloat pX, jfloat pY, jfloat pRadius, jfloat pDensity, jfloat pRestitution, jfloat pFriction, jboolean pFixedRotation, jboolean pHandleContacts){
		b2BodyDef bodyDef;
		bodyDef.position.Set(pX, pY);
		bodyDef.fixedRotation = pFixedRotation;
		
		b2Body* body = WORLD->CreateBody(&bodyDef);
		BodyIndexBodyData* bodyData = new BodyIndexBodyData;
		bodyData->bodyIndex = BODYINDEX;
		body->SetUserData(bodyData);

		// Define the circle shape.
		b2CircleDef circleDef;
		circleDef.radius = pRadius;
		circleDef.density = pDensity;
		circleDef.restitution = pRestitution;
		circleDef.friction = pFriction;

		body->CreateShape(&circleDef);
		body->SetMassFromShapes();
		
		BODIES[BODYINDEX] = body;
		BODIES_CONTACT_ENABLED[BODYINDEX] = pHandleContacts;
		
		return BODYINDEX++;
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    createBox
	 * Signature: (FFFFFFF)I
	 */
	JNIEXPORT jint JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_createBox (JNIEnv *pEnvironment, jobject pCaller, jfloat pX, jfloat pY, jfloat pWidth, jfloat pHeight, jfloat pDensity, jfloat pRestitution, jfloat pFriction, jboolean pFixedRotation, jboolean pHandleContacts){
		b2BodyDef bodyDef;
		bodyDef.position.Set(pX, pY);
		bodyDef.fixedRotation = pFixedRotation;

		b2Body* body = WORLD->CreateBody(&bodyDef);
		BodyIndexBodyData* bodyData = new BodyIndexBodyData();
		bodyData->bodyIndex = BODYINDEX;
		body->SetUserData(bodyData);

		// Define the box shape.
		b2PolygonDef boxDef;
		boxDef.density = pDensity;
		boxDef.restitution = pRestitution;
		boxDef.friction = pFriction;

		// The extents are the half-widths of the box.
		boxDef.SetAsBox(pWidth/2, pHeight/2);

		body->CreateShape(&boxDef);
		body->SetMassFromShapes();
		
		BODIES[BODYINDEX] = body;
		BODIES_CONTACT_ENABLED[BODYINDEX] = pHandleContacts;
		
		return BODYINDEX++;
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    destroyBody
	 * Signature: (I)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_destroyBody (JNIEnv *pEnvironment, jobject pCaller, jint pBodyIndex){
		if(BODIES[pBodyIndex] != NULL){
			delete ((BodyIndexBodyData*)BODIES[pBodyIndex]->GetUserData());
			WORLD->DestroyBody(BODIES[pBodyIndex]);
			BODIES[pBodyIndex] = NULL;
		}
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    getBodyInfo
	 * Signature: (Lcom/akjava/android/box2d/BodyInfo;I)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_getBodyInfo (JNIEnv* pEnvironment, jobject pCaller, jobject pBodyInfo, jint pBodyIndex){
		jclass bodyInfoClass = pEnvironment->GetObjectClass(pBodyInfo);
		jmethodID setValuesId = pEnvironment->GetMethodID(bodyInfoClass, "setValues", "(FFF)V");
		b2Body *body = BODIES[pBodyIndex];
		pEnvironment->CallVoidMethod(pBodyInfo, setValuesId, body->GetPosition().x, body->GetPosition().y, body->GetAngle());
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    setGravity
	 * Signature: (FF)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_setGravity (JNIEnv *pEnvironment, jobject pCaller, jfloat pGravityX, jfloat pGravityY){
		b2Vec2 gravity(pGravityX, pGravityY);
		WORLD->SetGravity(gravity);
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    setBodyXForm
	 * Signature: (IFFF)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_setBodyXForm (JNIEnv *pEnvironment, jobject pCaller, jint pBodyIndex, jfloat pX, jfloat pY, jfloat pAngle){
		if(BODIES[pBodyIndex] != NULL){
			b2Vec2 vec(pX, pY);
			BODIES[pBodyIndex]->SetXForm(vec, pAngle);
		}
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    setBodyAngularVelocity
	 * Signature: (IF)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_setBodyAngularVelocity (JNIEnv *pEnvironment, jobject pCaller, jint pBodyIndex, jfloat pAngle){
		if(BODIES[pBodyIndex] != NULL){
			BODIES[pBodyIndex]->SetAngularVelocity(pAngle);
		}
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    setBodyLinearVelocity
	 * Signature: (IFF)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_setBodyLinearVelocity (JNIEnv *pEnvironment, jobject pCaller, jint pBodyIndex, jfloat pX, jfloat pY){
		if(BODIES[pBodyIndex] != NULL){
			b2Vec2 vec(pX, pY);
			BODIES[pBodyIndex]->SetLinearVelocity(vec);
		}
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    getStatus
	 * Signature: (Lcom/akjava/android/box2d/BodyInfo;I)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_getStatus (JNIEnv *pEnvironment, jobject pCaller, jobject pBodyInfo, jint pBodyIndex){
		jclass bodyInfoClass = pEnvironment->GetObjectClass(pBodyInfo);
		jmethodID setValuesId = pEnvironment->GetMethodID(bodyInfoClass, "setStatus", "(ZZZZZ)V");
		b2Body *body = BODIES[pBodyIndex];
		pEnvironment->CallVoidMethod(pBodyInfo, setValuesId, body->IsBullet(), body->IsSleeping(), body->IsFrozen(),body->IsDynamic(),body->IsStatic());
	}

	/*
	 * Class:     org_anddev_andengine_physics_box2d_Box2DNativeWrapper
	 * Method:    getLinearVelocity
	 * Signature: (Lcom/akjava/android/box2d/BodyInfo;I)V
	 */
	JNIEXPORT void JNICALL Java_org_anddev_andengine_physics_box2d_Box2DNativeWrapper_getLinearVelocity (JNIEnv *pEnvironment, jobject pCaller, jobject pBodyInfo, jint pBodyIndex){
		jclass bodyInfoClass = pEnvironment->GetObjectClass(pBodyInfo);
		jmethodID setValuesId = pEnvironment->GetMethodID(bodyInfoClass, "setLinearVelocity", "(FF)V");
		b2Body *body = BODIES[pBodyIndex];
		pEnvironment->CallVoidMethod(pBodyInfo, setValuesId, body->GetLinearVelocity().x, body->GetLinearVelocity().y);
	}
}