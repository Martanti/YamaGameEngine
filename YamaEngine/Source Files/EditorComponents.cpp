#include "EditorComponents.h";
#ifdef EDITOR

nlohmann::json EditorComponentBase::GetJsonedObject()
{
	Log(LGR_WARNING, "Base function jsonification of the editor component has been called");
	return nlohmann::json();
}

void EditorComponentBase::PopulateFromJson(nlohmann::json const& jObject)
{
	Log(LGR_WARNING, "Base json->editor component converter has been called");
}

std::string EditorComponentBase::GetTypeName()
{
	switch (ComponentType)
	{
		case EditorTransformType:
			return "Transform";

		case EditorMeshType:
			return "Mesh";

		case EditorColliderType:
			return "Collider";

		case EditorGameScriptType:
			return "Game scripts";

		case EditorAudioSourcesType:
			return "Audio source";

		case EditorCameraType:
			return "Camera";

		case EditorLightType:
			return "Light";
		case EditorPathNavType:
			return "AINavigation";
		default:
			return "Unknown";
	}
}

#pragma region Transform
TransformEditorComponent::TransformEditorComponent()
{
	ComponentType = EditorTransformType;
}

nlohmann::json TransformEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "Transform";
	{
		json positionObj;
		positionObj.emplace_back(mPos.X);
		positionObj.emplace_back(mPos.Y);
		positionObj.emplace_back(mPos.Z);

		jObject["Position"] = positionObj;
	}

	{
		json rotationObj;
		rotationObj.emplace_back(mRot.X);
		rotationObj.emplace_back(mRot.Y);
		rotationObj.emplace_back(mRot.Z);

		jObject["Rotation"] = rotationObj;
	}

	{
		json scaleObj;
		scaleObj.emplace_back(mScale.X);
		scaleObj.emplace_back(mScale.Y);
		scaleObj.emplace_back(mScale.Z);

		jObject["Scale"] = scaleObj;
	}

	jObject["Tag"] = mTag;
	jObject["Name"] = mName;

	return jObject;
}

void TransformEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		{ // Position
			auto& position = jObject["Position"];
			mPos.X = position.at(0).get<float>();
			mPos.Y = position.at(1).get<float>();
			mPos.Z = position.at(2).get<float>();
		}

		{ // Rotation
			auto& rotation = jObject["Rotation"];
			mRot.X = rotation.at(0).get<float>();
			mRot.Y = rotation.at(1).get<float>();
			mRot.Z = rotation.at(2).get<float>();
		}

		{ // Scale
			auto& scale = jObject["Scale"];
			mScale.X = scale.at(0).get<float>();
			mScale.Y = scale.at(1).get<float>();
			mScale.Z = scale.at(2).get<float>();
		}

		mName = jObject["Name"].get<std::string>();
		mTag = jObject["Tag"].get<std::string>();
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from transform json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}
#pragma endregion

#pragma region Graphics
GraphicsEditorComponent::GraphicsEditorComponent()
{
	ComponentType = EditorMeshType;
}

nlohmann::json GraphicsEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "Mesh";
	jObject["Path"] = mMeshPath;
	jObject["TexturePath"] = mTexturePath;

	json animations;
	for (auto& [Speed, FrameStart, FrameEnd, Name] : mAnimations)
	{
		json animationData;

		animationData["Name"] = Name;
		animationData["Speed"] = Speed;
		animationData["StartFrame"] = FrameStart;
		animationData["EndFrame"] = FrameEnd;

		animations.emplace_back(animationData);
	}

	jObject["Animations"] = animations;
	return jObject;
}

void GraphicsEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		mMeshPath = jObject["Path"].get<std::string>();
		mTexturePath = jObject["TexturePath"].get<std::string>();

		auto animationData = jObject["Animations"];
		for (auto& item : animationData)
		{
			auto name = item["Name"].get<std::string>();
			auto speed = item["Speed"].get<float>();
			auto frameStart = item["StartFrame"].get<int>();
			auto frameEnd = item["EndFrame"].get<int>();

			mAnimations.emplace_back(AnimationData{ .Speed = speed,.FrameStart = frameStart, .FrameEnd = frameEnd, .Name = name });
		}

	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from graphics json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}
#pragma endregion

#pragma region Physics
PhysicsEditorComponent::PhysicsEditorComponent()
{
	// Dirty flag is not set as physics are not executed within the editor system
	ComponentType = EditorColliderType;
}

nlohmann::json PhysicsEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "Physics";
	jObject["ColliderType"] = mColliderType;
	jObject["Mass"] = mMass;

	{
		json extenObj;

		extenObj.emplace_back(mExtents.X);
		extenObj.emplace_back(mExtents.Y);
		extenObj.emplace_back(mExtents.Z);

		jObject["Extents"] = extenObj;
	}

	{
		json offsetObj;
		offsetObj.emplace_back(mOffsets.X);
		offsetObj.emplace_back(mOffsets.Y);
		offsetObj.emplace_back(mOffsets.Z);

		jObject["Offset"] = offsetObj;
	}

	return jObject;
}

void PhysicsEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		mColliderType = jObject["ColliderType"].get<std::string>();
		mMass = jObject["Mass"].get<float>();

		{ // Offset
			auto& offsets = jObject["Offset"];
			mOffsets.X = offsets.at(0).get<float>();
			mOffsets.Y = offsets.at(1).get<float>();
			mOffsets.Z = offsets.at(2).get<float>();
		}

		{ // Extents
			auto& extents = jObject["Extents"];
			mExtents.X = extents.at(0).get<float>();
			mExtents.Y = extents.at(1).get<float>();
			mExtents.Z = extents.at(2).get<float>();
		}
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from physics json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}
#pragma endregion


#pragma region Scripts
ScriptsEditorComponent::ScriptsEditorComponent()
{
	ComponentType = EditorGameScriptType;
}


nlohmann::json ScriptsEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "Script";

	json scriptNameArr;
	for (auto& scriptName : mScriptContainer)
	{
		scriptNameArr.emplace_back(scriptName);
	}

	jObject["ScriptContainer"] = scriptNameArr;

	return jObject;
}

void ScriptsEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		auto& scripts = jObject["ScriptContainer"];
		for (auto& item : scripts)
		{
			mScriptContainer.emplace_back(item.get<std::string>());
		}
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from scripts json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}
#pragma endregion


#pragma region Audio

AudioEditorComponent::AudioEditorComponent()
{
	ComponentType = EditorAudioSourcesType;
}

nlohmann::json AudioEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "Audio";
	json jSources;
	for (auto& [name, path] : mAudioSources)
	{
		json jAudio;
		jAudio["Name"] = name;
		jAudio["Path"] = path;
		jSources.emplace_back(jAudio);
	}

	jObject["Sources"] = jSources;
	return jObject;
}

void AudioEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		auto& sources = jObject["Sources"];

		for (auto& item : sources)
		{
			auto name = item["Name"].get<std::string>();
			auto path = item["Path"].get<std::string>();

			mAudioSources.emplace_back(std::make_pair(name, path));
		}
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from audio sources json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}

#pragma endregion

#pragma region Camera

CameraEditorComponent::CameraEditorComponent()
{
	ComponentType = EditorCameraType;
}

nlohmann::json CameraEditorComponent::GetJsonedObject()
{
	json jObject;

	jObject["Type"] = "Camera";
	jObject["CamType"] = mCamType;

	json offset;
	offset.emplace_back(mOffsets.X);
	offset.emplace_back(mOffsets.Y);
	offset.emplace_back(mOffsets.Z);

	jObject["Offset"] = offset;

	return jObject;
}

void CameraEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		mCamType = jObject["CamType"].get<std::string>();

		auto& offset = jObject["Offset"];

		mOffsets.X = offset.at(0).get<float>();
		mOffsets.Y = offset.at(1).get<float>();
		mOffsets.Z = offset.at(2).get<float>();

	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from camera json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}
#pragma endregion

#pragma region Light
LightEditorComponent::LightEditorComponent()
{
	ComponentType = EditorLightType;
}

nlohmann::json LightEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "Light";
	jObject["LightType"] = mLightType;
	jObject["Range"] = mRange;

	{
		json diffuse;
		diffuse.emplace_back(mDiffuseR);
		diffuse.emplace_back(mDiffuseG);
		diffuse.emplace_back(mDiffuseB);
		diffuse.emplace_back(mDiffuseA);
		jObject["DiffuseColour"] = diffuse;
	}

	{
		json ambient;
		ambient.emplace_back(mAmbientR);
		ambient.emplace_back(mAmbientG);
		ambient.emplace_back(mAmbientB);
		ambient.emplace_back(mAmbientA);
		jObject["AmbientColour"] = ambient;
	}

	return jObject;
}

void LightEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		mLightType = jObject["LightType"].get<std::string>();
		mRange = jObject["Range"].get<float>();
		{
			auto diffuseJson = jObject["DiffuseColour"];
			mDiffuseR = diffuseJson.at(0).get<float>();
			mDiffuseG = diffuseJson.at(1).get<float>();
			mDiffuseB = diffuseJson.at(2).get<float>();
			mDiffuseA = diffuseJson.at(3).get<float>();
		}
		{
			auto ambientJson = jObject["AmbientColour"];
			mAmbientR = ambientJson.at(0).get<float>();
			mAmbientG = ambientJson.at(1).get<float>();
			mAmbientB = ambientJson.at(2).get<float>();
			mAmbientA = ambientJson.at(3).get<float>();
		}
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from light json: {}", e.what()));
		Log(LGR_WARNING, "Used data might be mixed between the default data and read data");
	}
}
#pragma endregion

#pragma region Navigation
BeaconNavEditorComponent::BeaconNavEditorComponent()
{
	ComponentType = EditorPathNavType;
}

nlohmann::json BeaconNavEditorComponent::GetJsonedObject()
{
	json jObject;
	jObject["Type"] = "AINavigation";
	json list;
	for (auto& item : mBeacons)
	{
		json position;

		position.emplace_back(item.X);
		position.emplace_back(item.Y);
		position.emplace_back(item.Z);

		list.emplace_back(position);
	}

	jObject["Points"] = list;
	return jObject;
}
void BeaconNavEditorComponent::PopulateFromJson(nlohmann::json const& jObject)
{
	try
	{
		auto& listOfPoints = jObject["Points"];

		for (auto& item : listOfPoints)
		{
			auto x = item.at(0).get<float>();
			auto y = item.at(1).get<float>();
			auto z = item.at(2).get<float>();
			mBeacons.emplace_back(x, y, z);
		}
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception while populating items from Beacon Navigation json: {}", e.what()));
		Log(LGR_WARNING, "The list of points might be returned incomplete or empty.");
	}

}
#pragma endregion


#endif // EDITOR