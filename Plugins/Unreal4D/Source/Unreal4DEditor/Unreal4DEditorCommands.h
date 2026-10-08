#pragma once

#include "Framework/Commands/Commands.h"

struct FUnreal4DRemovedCommandAndChord
{
	TSharedPtr<FUICommandInfo> Command;
	FInputChord OriginalChord;
	EMultipleKeyBindingIndex ChordIndex;

	bool operator!=(const FUnreal4DRemovedCommandAndChord& InOther) const
	{
		return Command != InOther.Command || OriginalChord != InOther.OriginalChord || ChordIndex != InOther.ChordIndex;
	}
};

class FUnreal4DEditorCommands : public TCommands<FUnreal4DEditorCommands>
{
	static TArray<FUnreal4DRemovedCommandAndChord> _removedNavChords;
	static bool CheckIfRemovedChordArraysMatch(const TArray<FUnreal4DRemovedCommandAndChord>& InA, const TArray<FUnreal4DRemovedCommandAndChord>& InB);

public:
	FUnreal4DEditorCommands();

	static FInputChord GetEffectiveActiveChord(const TSharedPtr<FUICommandInfo>& InCommand, EMultipleKeyBindingIndex InChordIndex);
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> WAxisPositive;
	TSharedPtr<FUICommandInfo> WAxisNegative;
	TSharedPtr<FUICommandInfo> Reset4DSlice;
	TSharedPtr<FUICommandInfo> Rotate4DModifier;

	// Called from Tick to dynamically block/restore viewport navigation chords
	// that conflict with our 4D keybindings. Re-evaluates when bindings change.
	static void UpdateViewportNavChordBlocking();
	// Called from module ShutdownModule to restore all removed chords.
	static void RestoreAllViewportNavChords();
};
