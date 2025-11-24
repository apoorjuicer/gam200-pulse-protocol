#pragma once

namespace ScriptAPI
{
	// Wrapper function for modifier and accessor for TransformComponent SRT
	public value struct TransformComponent
	{
	public:
		property float ScaleX
		{
			float get();
			void set(float value);
		}

		property float ScaleY
		{
			float get();
			void set(float value);
		}

		property float RotX
		{
			float get();
			void set(float value);
		}

		property float RotY
		{
			float get();
			void set(float value);
		}

		property float X
		{
			float get();
			void set(float value);
		}

		property float Y
		{
			float get();
			void set(float value);
		}

	internal:
		//Constrcutor when GetTransform() with Entity ID gets called
		TransformComponent(unsigned int ID);
	private:
		// the ID that this instance has
		unsigned int entityID;
	};

}