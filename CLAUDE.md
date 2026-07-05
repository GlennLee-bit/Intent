# Superpowers — Project Instructions

You have superpowers installed. The skills are located at `C:\Users\18800\.claude\plugins\superpowers\skills\`.

## Available Skills

| Skill | Path | When to Use |
|-------|------|-------------|
| brainstorming | skills/brainstorming/SKILL.md | Before any creative/implementation work. MUST use before writing code. |
| systematic-debugging | skills/systematic-debugging/SKILL.md | When fixing bugs. MUST use before debugging. |
| test-driven-development | skills/test-driven-development/SKILL.md | During implementation. Enforces RED-GREEN-REFACTOR. |
| writing-plans | skills/writing-plans/SKILL.md | After design approval. Breaks work into bite-sized tasks. |
| subagent-driven-development | skills/subagent-driven-development/SKILL.md | With approved plan. Dispatches subagent per task. |
| executing-plans | skills/executing-plans/SKILL.md | With approved plan. Batch execution with checkpoints. |
| dispatching-parallel-agents | skills/dispatching-parallel-agents/SKILL.md | When tasks can run concurrently. |
| requesting-code-review | skills/requesting-code-review/SKILL.md | Between tasks. Reviews against plan. |
| receiving-code-review | skills/receiving-code-review/SKILL.md | When receiving feedback. |
| using-git-worktrees | skills/using-git-worktrees/SKILL.md | After design approval. Creates isolated workspace. |
| finishing-a-development-branch | skills/finishing-a-development-branch/SKILL.md | When tasks complete. Verify, merge/PR/keep/discard. |
| verification-before-completion | skills/verification-before-completion/SKILL.md | Before declaring work done. Ensure it's actually fixed. |
| writing-skills | skills/writing-skills/SKILL.md | When creating new skills. |

## The Rule

**Invoke relevant skills BEFORE any response or action** — including clarifying questions, exploring the codebase, or checking files. If a skill might apply (even 1% chance), read and follow the skill file first.

**Before entering plan mode:** if you haven't already brainstormed, read the brainstorming skill first.

Announce "Using [skill] to [purpose]" and follow the skill exactly. If it has a checklist, create a TodoWrite todo per item.

## Skill Priority

Process skills come first — they set the approach, then implementation skills carry it out:

- "Let's build X" → read brainstorming skill first, then implementation skills.
- "Fix this bug" → read systematic-debugging skill first, then domain skills.

## Red Flags

These thoughts mean STOP — you're rationalizing:

| Thought | Reality |
|---------|---------|
| "This is just a simple question" | Questions are tasks. Check for skills. |
| "I need more context first" | Skill check comes BEFORE clarifying questions. |
| "Let me explore the codebase first" | Skills tell you HOW to explore. Check first. |
| "This doesn't need a formal skill" | If a skill exists, use it. |
| "I'll just do this one thing first" | Check BEFORE doing anything. |

## How to Read Skills

When a skill applies, read the SKILL.md file at `C:\Users\18800\.claude\plugins\superpowers\skills\<skill-name>\SKILL.md` using the Read tool, then follow its instructions exactly.
