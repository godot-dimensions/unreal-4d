#include "Unreal4DEditorCommands.h"

#define LOCTEXT_NAMESPACE "Unreal4DEditorCommands"

TArray<FUnreal4DRemovedCommandAndChord> FUnreal4DEditorCommands::_removedNavChords;

FUnreal4DEditorCommands::FUnreal4DEditorCommands()
	: TCommands<FUnreal4DEditorCommands>(
		TEXT("Unreal4DEditor"),
		NSLOCTEXT("Contexts", "Unreal4DEditor", "Unreal 4D Editor"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FUnreal4DEditorCommands::RegisterCommands()
{
	UI_COMMAND(WAxisPositive, "Unreal 4D W Axis Positive", "Move the 4D camera in the positive W direction", EUserInterfaceActionType::Button, FInputChord(EKeys::E));
	UI_COMMAND(WAxisNegative, "Unreal 4D W Axis Negative", "Move the 4D camera in the negative W direction", EUserInterfaceActionType::Button, FInputChord(EKeys::Q));
	UI_COMMAND(Reset4DSlice, "Unreal 4D Reset 4D Slice", "Reset the 4D camera slice to the default orientation", EUserInterfaceActionType::Button, FInputChord(EKeys::P));
	UI_COMMAND(Rotate4DModifier, "Unreal 4D Rotate 4D Modifier", "Hold alongside right-click camera look to rotate in 4D", EUserInterfaceActionType::Button, FInputChord(EKeys::LeftMouseButton), FInputChord(EKeys::MiddleMouseButton));
}

FInputChord FUnreal4DEditorCommands::GetEffectiveActiveChord(const TSharedPtr<FUICommandInfo>& InCommand, EMultipleKeyBindingIndex InChordIndex)
{
	if (!InCommand.IsValid())
	{
		return FInputChord();
	}
	const FInputChord ActiveChord = InCommand->GetActiveChord(InChordIndex).Get();
	if (InCommand == FUnreal4DEditorCommands::Get().Rotate4DModifier)
	{
		// Special case: The default chord for Rotate4DModifier is Left Mouse Button.
		// However, Unreal's editor keyboard shotcuts menu does not allow binding mouse buttons.
		// Therefore, unbinding the Left Mouse Button from Rotate4DModifier will make it impossible
		// to re-bind Left Mouse Button back to it. Work-around: Use Left Mouse Button when blank.
		if (InChordIndex == EMultipleKeyBindingIndex::Primary && !ActiveChord.Key.IsValid())
		{
			return FInputChord(EKeys::LeftMouseButton);
		}
	}
	return ActiveChord;
}

bool FUnreal4DEditorCommands::CheckIfRemovedChordArraysMatch(const TArray<FUnreal4DRemovedCommandAndChord>& InA, const TArray<FUnreal4DRemovedCommandAndChord>& InB)
{
	const int32 amountA = InA.Num();
	if (amountA != InB.Num())
	{
		return false;
	}
	for (int32 i = 0; i < amountA; i++)
	{
		if (InA[i] != InB[i])
		{
			return false;
		}
	}
	return true;
}

// When one of the 4D keybindings conflicts with one of the built-in
// Unreal editor viewport navigation keybindings, we need to disable
// the conflicting 3D navigation keybinding to avoid having 2 keys
// doing different things at the same time.
//
// Unreal includes some keybindings that "conflict" with each other,
// such as W for translate and W for move forward, but these depend
// on a differing context, namely right-click. The default bindings
// of Q for World Down and Q for W Axis Negative don't have such a
// context difference, they both are intended to be active at the
// same time, namely when right-click is held. Therefore, we need to
// completely unbind the conflicting chords when any 4D controls are
// assigned to the same keys, and if the user changes the keybindings,
// we need to re-bind and re-unbind as necessary.
//
// This runs every tick, called by `FUnreal4DEditorMode::Tick`, so that if
// the keybinds are changed later, the blocking will update accordingly.
void FUnreal4DEditorCommands::UpdateViewportNavChordBlocking()
{
	// First, collect all keys currently used by our 4D commands.
	const FUnreal4DEditorCommands& commands = FUnreal4DEditorCommands::Get();
	TSet<FKey> current4DKeys;
	for (const TSharedPtr<FUICommandInfo>& Cmd : { commands.WAxisPositive, commands.WAxisNegative, commands.Reset4DSlice, commands.Rotate4DModifier })
	{
		const FInputChord Primary = GetEffectiveActiveChord(Cmd, EMultipleKeyBindingIndex::Primary);
		if (Primary.Key.IsValid())
		{
			current4DKeys.Add(Primary.Key);
		}
		const FInputChord Secondary = GetEffectiveActiveChord(Cmd, EMultipleKeyBindingIndex::Secondary);
		if (Secondary.Key.IsValid())
		{
			current4DKeys.Add(Secondary.Key);
		}
	}
	// We also need to collect keys from all possible conflicting Unreal 3D
	// navigation commands, meaning, everything active when holding right-click.
	const FViewportNavigationCommands& navCommands = FViewportNavigationCommands::Get();
	const TArray<TSharedPtr<FUICommandInfo>> allNavCommands = {
		navCommands.Forward, navCommands.Backward, navCommands.Left, navCommands.Right,
		navCommands.WorldUp, navCommands.WorldDown, navCommands.LocalUp, navCommands.LocalDown,
		navCommands.FovZoomIn, navCommands.FovZoomOut,
		navCommands.RotateUp, navCommands.RotateDown, navCommands.RotateLeft, navCommands.RotateRight,
	};
	// Next, build the list of all Unreal editor viewport navigation commands
	// and their "effective" chords (both actual active, and removed chords).
	// This is needed to determine which of Unreal's 3D navigation chords are
	// desired/supposed to be disabled/removed. If this set changes, such as
	// a user re-binding a navigation command to a different key, then we
	// need to update the blocking to match.
	TArray<FUnreal4DRemovedCommandAndChord> desiredRemovals;
	for (const TSharedPtr<FUICommandInfo>& navCommand : allNavCommands)
	{
		if (!navCommand.IsValid())
		{
			continue;
		}
		for (EMultipleKeyBindingIndex chordIdx : { EMultipleKeyBindingIndex::Primary, EMultipleKeyBindingIndex::Secondary })
		{
			// Determine the effective chord: if we already removed this one,
			// use the cached original; otherwise use the live active chord.
			const FUnreal4DRemovedCommandAndChord* existingRemoval = _removedNavChords.FindByPredicate(
				[&](const FUnreal4DRemovedCommandAndChord& remNavChord) { return remNavChord.Command == navCommand && remNavChord.ChordIndex == chordIdx; });
			const FInputChord& effectiveChord = existingRemoval ? existingRemoval->OriginalChord : navCommand->GetActiveChord(chordIdx).Get();
			if (effectiveChord.Key.IsValid() && current4DKeys.Contains(effectiveChord.Key))
			{
				desiredRemovals.Add({ navCommand, effectiveChord, chordIdx });
			}
		}
	}
	// Compare desired removals against current removals.
	// If they match exactly, nothing has changed and we can skip.
	if (CheckIfRemovedChordArraysMatch(desiredRemovals, _removedNavChords))
	{
		return;
	}
	// If we haven't returned, that means we need to re-bind (restore)
	// and re-unbind (remove) everything. Start by restoring/re-binding.
	RestoreAllViewportNavChords();
	// Remove all chords that conflict with our 4D keys.
	for (const FUnreal4DRemovedCommandAndChord& desired : desiredRemovals)
	{
		if (desired.Command.IsValid())
		{
			_removedNavChords.Add(desired);
			desired.Command->RemoveActiveChord(desired.ChordIndex);
		}
	}
}

void FUnreal4DEditorCommands::RestoreAllViewportNavChords()
{
	for (const FUnreal4DRemovedCommandAndChord& removedNavChord : _removedNavChords)
	{
		if (removedNavChord.Command.IsValid())
		{
			removedNavChord.Command->SetActiveChord(removedNavChord.OriginalChord, removedNavChord.ChordIndex);
		}
	}
	_removedNavChords.Reset();
}

#undef LOCTEXT_NAMESPACE
