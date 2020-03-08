Before opening the PR, think: 
- Is this PR bulletproof? Can I think of no ways to improve this PR beyond its current state?
- Is the PR short enough to ensure a quick but thorough review, but long enough to represent a complete feature? For large state machines, let's try opening PRs for only a few states at a time.

If the answer to either of these questions is "no", refactor your code and open your PR when the answer to both questions is "yes".

--------------------------

# PR Title

Fixes #. (Optional; sometimes PRs don't have an associated ticket)

### Summary of changes
- 
- 
- 

### Testing
Such as your unit and functional testing. The testing should be at a level appropriate to demonstrate that your feature is sufficiently tested.
Post a HITL log if the test could affect HITL (i.e. major changes to subsystem controllers or to mission manager).
Post a HOOTL log if it helps support the claim that the master branch will remain unbroken.

If your update does not require significant testing, argue why, or if you're deferring testing to another PR, create an issue ticket for the deferral and link it.

### Constants
If your PR creates any constants, list them here and indicate whether or not you've added them to the recurring constants review.

### Documentation Evidence
Post to ReadTheDocs, or
- Argue that your inline documentation is sufficient.
- Create a issue ticket deferring the documentation task.
