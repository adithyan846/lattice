# CLAUDE.md

# Obsidian Vault Configuration — Lattice Flight Controller

## Formatting Rules

1. Every note is an atomic Markdown page in `wiki/`
2. Use `[[Double Bracket Links]]` to connect related notes
3. Every note must have at least one incoming link (no orphans)
4. Use YAML frontmatter for metadata:
   ```yaml
   ---
   tags: [architecture, makefile, phase-1]
   status: draft
   created: 2026-11-01
   ---
   ```
5. Use Mermaid diagrams for architecture visualization
6. Link back to `_index.md` from every top-level note

## Naming Conventions

- Notes: `Topic Name.md` (Title Case, spaces)
- Tags: lowercase, no spaces, e.g. `#architecture #makefile #phase-1`
- Links: `[[Topic Name]]` or `[[Topic Name|display text]]`

## Graph Strategy

- Global graph shows all notes in `wiki/`
- Local graph shows 1-2 degrees of separation from active note
- Filter by tag to view clusters (e.g., `#architecture` vs `#makefile`)

## LLM Instructions

When updating this knowledge base:
1. Read source files in `raw/` first
2. Extract concepts into atomic `wiki/` pages
3. Create `[[bi-directional links]]` between related concepts
4. Update `_index.md` with new entries
5. Do not duplicate information — link instead
