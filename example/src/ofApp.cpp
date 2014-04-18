#include "ofMain.h"

#include "ofxNodeGraph.h"

class MySubNode : public ofxNodeGraph::Node
{
public:
	~MySubNode() { cout << "delete: " << getName() << endl; }
	
	void setup() { setGlobalPosition(ofVec3f(200, 0, 0)); }

	void update()
	{
		rotate(1, ofVec3f(0, 1, 0));
		rotate(1, ofVec3f(0, 0, 1));
	}

	void draw()
	{
		ofDrawBox(30);
		ofDrawAxis(30);
		
		ofDrawBitmapString(getName(), 30, 0);
	}
};

class MyNode : public ofxNodeGraph::Node
{
public:
	~MyNode() { cout << "delete: " << getName() << endl; }

	MyNode* setup()
	{
		setScale(ofVec3f(0.5, 0.5, 0.5));

		addChild<MySubNode>("MySubNode")->setup();

		return this;
	}

	void update() { rotate(1, ofVec3f(0, 0, 1)); }

	void draw()
	{
		ofDrawAxis(100);
		ofDrawBitmapString(getName(), 30, 0);

		ofVec3f local = ofVec3f(0, 0, 0);
		
		// calc node to node transform
		ofMatrix4x4 m = getNodeToNodeTransform(at<MySubNode>(0), this);
		ofLine(local, m.preMult(local));
		
		ofPushStyle();
		ofSetColor(255, 255, 0);
		ofDrawBox(m.preMult(local), 100);
		ofPopStyle();
	}
};

class ofApp : public ofBaseApp
{
public:
	ofxNodeGraph::RootNode root;

	void setup()
	{
		ofSetFrameRate(0);
		ofSetVerticalSync(true);
		ofBackground(0);

		for (int i = 0; i < 10; i++)
		{
			// create & setup
			root.addChild<MyNode>("MyNode" + ofToString(i))
				->setup()
				->setPosition(ofVec3f(0, i * 100, 0));
		}
	}

	void update() { root.update(); }

	ofEasyCam cam;

	void draw()
	{
		ofNoFill();

		cam.begin();
		root.draw();
		cam.end();

		ofDrawBitmapString(ofToString(ofGetFrameRate(), 1), 10, 20);
	}

	void keyPressed(int key) { root.clear(); }

	void keyReleased(int key) {}

	void mouseMoved(int x, int y) {}

	void mouseDragged(int x, int y, int button) {}

	void mousePressed(int x, int y, int button) {}

	void mouseReleased(int x, int y, int button) {}

	void windowResized(int w, int h) {}
};


int main(int argc, const char** argv)
{
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp);
	return 0;
}
